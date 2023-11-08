#include "Parser.hpp"

#include <unistd.h>

#include <boost/algorithm/string.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <ncurses.h>

#if USE_OMP
#include <omp.h>
#endif

void Parser::parse() {
    // Determine the number of threads that can run concurrently
    uint64_t numThreads = std::thread::hardware_concurrency();
    std::cout << "Your computer supports " << numThreads << " concurrent threads.\n";
#ifdef USE_OMP
    std::cout << "Using OpenMP.\n";
#endif

    std::ifstream infile(filename);
    std::string line;
    VcdScope* curr_scope = nullptr;
    std::string token;

    std::string version;
    std::string date;
    std::string timescale;

    startMeasureTime();

    while (std::getline(infile, line)) {
        boost::algorithm::trim(line);
        if (line.empty()) continue;

        std::istringstream ss(line);

        while (ss >> token) {
            if (token.at(0) == '$') {
                curr_state = getParseState(token);
                if (curr_state == PARSE_UPSCOPE) {
                    curr_scope = curr_scope->parent;
                    break;
                }
            } else {
                switch (curr_state) {
                    case PARSE_VERSION: {
                        version += ' ' + token;
                        break;
                    }
                    case PARSE_DATE: {
                        date += ' ' + token;
                        break;
                    }
                    case PARSE_TIMESCALE: {
                        timescale += ' ' + token;
                        break;
                    }
                    case PARSE_SCOPE: {
                        VcdScope* next_scope = new VcdScope();
                        next_scope->parent = curr_scope;
                        scopes.emplace_back(next_scope);
                        ss >> token;
                        next_scope->name = token;
                        if (!curr_scope) // top level
                        {
                            if (!top_scope) 
                            {
                                top_scope = next_scope;
                                curr_scope = next_scope;
                            } 
                            else 
                            { // visited before
                                curr_scope = top_scope;
                            }
                            
                        }   
                        else if (!curr_scope->children.count(next_scope->name)) // non-top level, haven't visited before
                        {
                            curr_scope->children[next_scope->name] = next_scope;
                            curr_scope = next_scope;
                        }
                        else // top level, have visited before
                            curr_scope = (VcdScope*) curr_scope->children[next_scope->name];
                        break;
                    }
                    case PARSE_VAR: {
                        std::string type = token;
                        ss >> token;
                        uint64_t size = stoi(token);
                        ss >> token;
                        std::string hash = token;
                        ss >> token;
                        std::string name = token;
                        ss >> token;
                        std::string dimensions = token;
                        if (!var_map.count(hash)) {
                            VcdVar* curr_var = new VcdVar();
                            curr_var->parent = curr_scope;
                            curr_var->size = size;
                            curr_var->hash = hash;
                            curr_var->name = name;
                            curr_var->dimensions = dimensions;
                            var_map[hash] = curr_var;
                            var_hashes.push_back(hash);
                        }
                        curr_scope->children[var_map[hash]->name] = var_map[hash];
                        break;
                    }
                    case PARSE_VALUES: {
                        if (token.at(0) == '#') {
                            curr_time = stoi(token.substr(1, token.size()));
                        } else if (token.at(0) == 'b') {
                            std::string value = token;
                            ss >> token;
                            var_map[token]->vcd_values.emplace_back(
                                std::pair(curr_time, value));
                        } else {
                            std::string hash = token.substr(1, token.size());
                            var_map[hash]->vcd_values.emplace_back(
                                std::pair(curr_time, token.substr(0, 1)));
                        }
                        break;
                    }
                    default:
                        assert(false);
                }
            }
        }
    }

    endMeasureTime("Parse Time");
    
    startMeasureTime();

#ifdef USE_OMP
    constructValueIntervals();
#else
    std::list<std::thread> threads;
    uint64_t numVars = var_hashes.size();
    uint64_t varsPerThread = floor((numVars) / (double) (numThreads - 1)); // min # vars each thread should have
    uint64_t remainder = numVars % (numThreads - 1);
    uint64_t i = 0;
    // Create the threads (max n-1) and distribute responsibility among the threads
    while (threads.size() < (numThreads - 1)) 
    {
        uint64_t end = threads.size() >= remainder ? i + varsPerThread : i + varsPerThread + 1;
        threads.emplace_back(
            std::thread(
                [this, start = i, end]() 
                {
                    this->constructValueIntervals(start, end);
                }
            )
        );
        i = end;
    }

    // wait for threads to finish
    for (auto& thread : threads) 
    {
        thread.join();
    }
#endif
    endMeasureTime("Value Interval Processing Time");

    printw(
        "\n\r"
        "Version:   %s\n\r"
        "Date:      %s\n\r"
        "Timescale: %s\n\r"
        "Top scope: %s\n\r",
        version.c_str(),
        date.c_str(),
        timescale.c_str(),
        top_scope->name.c_str()
    );

}

#ifdef USE_OMP
void Parser::constructValueIntervals() {
    constructValueIntervals(0, var_hashes.size());
}
#endif

void Parser::constructValueIntervals(uint64_t startIdx, uint64_t endIdx) {
#ifdef USE_OMP
#pragma omp parallel for
#endif
    for (uint64_t i = startIdx; i < endIdx; i++) {
        std::string hash = var_hashes[i];
        VcdVar* var = var_map[hash];
        auto it = var->vcd_values.begin();
        uint64_t prev_timestamp = it->first;
        std::string prev_value = it->second;
        ++it;
        for (; it != var->vcd_values.end(); ++it) {
            var->interval_values +=
                std::make_pair(
                    boost::icl::interval<uint64_t>::right_open(
                        prev_timestamp, it->first
                    ),
                    prev_value);
            prev_timestamp = it->first;
            prev_value = it->second;
        }
        var->interval_values +=
            std::make_pair(
                boost::icl::interval<uint64_t>::right_open(
                    prev_timestamp, curr_time
                ),       
                prev_value);
    }
}

Parser::~Parser() {
    // delete scopes
    for (auto& scope : scopes) {
        delete scope;
    }
    // delete vars
    for (auto& x : var_map) {
        delete x.second;
    }
}

inline Parser::State Parser::getParseState(std::string token) {
    if (curr_state == PARSE_VALUES) return PARSE_VALUES;
    if (token == "$version") return PARSE_VERSION;
    if (token == "$date") return PARSE_DATE;
    if (token == "$timescale") return PARSE_TIMESCALE;
    if (token == "$scope") return PARSE_SCOPE;
    if (token == "$upscope") return PARSE_UPSCOPE;
    if (token == "$var") return PARSE_VAR;
    if (token == "$enddefinitions") return PARSE_ENDDEFINITIONS;
    if (token == "$end") {
        if (curr_state == PARSE_ENDDEFINITIONS) return PARSE_VALUES;
        return PARSE_NONE;
    }
    return PARSE_ERR;
}

VcdScope* Parser::getTop() {
    return top_scope;
}

VcdVar* Parser::getVcdVar(std::string hierarchicalName, VcdScope* scope) {
    size_t pos = hierarchicalName.find('.');
    std::string token;
    VcdScope* curr_scope = scope;
    token = hierarchicalName.substr(0, pos);
    assert(curr_scope->name == token);
    hierarchicalName.erase(0, pos + 1);
    while ((pos = hierarchicalName.find('.')) != std::string::npos) {
        token = hierarchicalName.substr(0, pos);
        curr_scope = (VcdScope*) curr_scope->children[token];
        hierarchicalName.erase(0, pos + 1);
    }
    return (VcdVar*) curr_scope->children[hierarchicalName];
}

VcdVar* Parser::getVcdVar(std::string hierarchicalName) {
    return getVcdVar(hierarchicalName, top_scope);
}

void Parser::startMeasureTime() {
    startTime = std::chrono::high_resolution_clock::now();
}

void Parser::endMeasureTime(std::string desc) {
    auto elapsed = std::chrono::high_resolution_clock::now() - startTime;
    printw("%s: %llu us\n\r", desc.c_str(), std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count());
}
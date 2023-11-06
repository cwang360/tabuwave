#include "Parser.hpp"

#include <unistd.h>

#include <boost/algorithm/string.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>

void Parser::parse() {
    // Determine the number of threads that can run concurrently
    std::list<std::thread> threads;
    uint64_t numThreads = std::thread::hardware_concurrency();
    std::cout << "Your computer supports " << numThreads << " concurrent threads.\n";

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
                        if (!curr_scope)
                            top_scope = next_scope;
                        else
                            curr_scope->children[next_scope->name] = next_scope;
                        curr_scope = next_scope;
                        break;
                    }
                    case PARSE_UPSCOPE: {
                        curr_scope = curr_scope->parent;
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
    uint64_t numVars = var_hashes.size();
    uint64_t varsPerThread = ceil((numVars) / (double) (numThreads - 1)); // for first n-2 threads
    uint64_t i = 1;
    // Create the threads (max n-1) and distribute responsibility among the threads
    for (; (i < numThreads - 1) && (i < numVars); i++) 
    {
        threads.emplace_back(std::thread([this, start = (i - 1) * varsPerThread, end = (i * varsPerThread)]() {this->constructValueIntervals(start, end);}));
    }
    threads.emplace_back(std::thread([this, start = (i - 1) * varsPerThread, end = numVars]() {this->constructValueIntervals(start, end);}));

    // wait for threads to finish
    for (auto& thread : threads) 
    {
        thread.join();
    }
    std::cout << numVars << "\n" << varsPerThread << std::endl;

    endMeasureTime("Value Interval Processing Time");
    // for (auto const& x : var_map) {
    //     VcdVar* var = x.second;
    //     auto it = var->vcd_values.begin();
    //     uint64_t prev_timestamp = it->first;
    //     std::string prev_value = it->second;
    //     ++it;
    //     for (; it != var->vcd_values.end(); ++it) {
    //         var->interval_values +=
    //             std::make_pair(boost::icl::interval<uint64_t>::right_open(
    //                                prev_timestamp, it->first),
    //                            prev_value);
    //         prev_timestamp = it->first;
    //         prev_value = it->second;
    //     }
    //     var->interval_values +=
    //         std::make_pair(boost::icl::interval<uint64_t>::right_open(
    //                            prev_timestamp, curr_time),
    //                        prev_value);
    // }
    
    std::cout << "Version: " << version << std::endl
              << "Date: " << date << std::endl
              << "Timescale: " << timescale << std::endl;

    std::cout << "Top scope: " << top_scope->name << std::endl;
    // for (auto child : top_scope->children) {
    //     std::cout << child.first;
    //     if (child.first == "clk") {
    //         std::cout << ((VcdVar*)child.second)->hash << std::endl;
    //         std::cout << child.second << std::endl;
    //         std::cout << var_map["{("] << std::endl;
    //         // for (auto x : ((VcdVar*) child)->vcd_values) {
    //         //     std::cout << x.first << std::endl;
    //         // }
    //         // for (auto x : var_map["{("]->vcd_values) {
    //         //     std::cout << x.first << ' ' << x.second << std::endl;
    //         // }
    //         auto it = ((VcdVar*)child.second)->interval_values.begin();
    //         while (it != ((VcdVar*)child.second)->interval_values.end()) {
    //             auto time = it->first;
    //             std::string value = (*it++).second;
    //             std::cout << time << ": " << value << std::endl;
    //         }
    //         std::cout << ((VcdVar*)child.second)->interval_values(399997) << std::endl;
    //     }
    // }
}

void Parser::constructValueIntervals(uint64_t startIdx, uint64_t endIdx) {
    for (unsigned int i = startIdx; i < endIdx; i++) {
        std::string hash = var_hashes[i];
        VcdVar* var = var_map[hash];
        auto it = var->vcd_values.begin();
        uint64_t prev_timestamp = it->first;
        std::string prev_value = it->second;
        ++it;
        for (; it != var->vcd_values.end(); ++it) {
            var->interval_values +=
                std::make_pair(boost::icl::interval<uint64_t>::right_open(
                                   prev_timestamp, it->first),
                               prev_value);
            prev_timestamp = it->first;
            prev_value = it->second;
        }
        var->interval_values +=
            std::make_pair(boost::icl::interval<uint64_t>::right_open(
                               prev_timestamp, curr_time),
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
        curr_scope = curr_scope->children[token];
        hierarchicalName.erase(0, pos + 1);
    }
    return (VcdVar*) curr_scope->children[hierarchicalName];
}

VcdVar* Parser::getVcdVar(std::string hierarchicalName) {
    return getVcdVar(hierarchicalName, top_scope);
};

void Parser::startMeasureTime() {
    startTime = std::chrono::high_resolution_clock::now();
}

void Parser::endMeasureTime(std::string desc) {
    auto elapsed = std::chrono::high_resolution_clock::now() - startTime;
    std::cout << desc << ": " << std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count() << " us" << std::endl;
}
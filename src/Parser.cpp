/**
 * Author:          Cynthia Wang
 * Date created:    10/27/2023
 * Organization:    ECE 4122
 *
 * Description:
 * Source file for Parser class and functions. See Parser.hpp for function descriptions.
*/

#include "Parser.hpp"

#include <unistd.h>

#include <boost/algorithm/string.hpp>
#include <fstream>
#include <iostream>
#include <sstream>
#include <thread>
#include <ncurses.h>
#include <regex>

#if USE_OMP
#include <omp.h>
#endif

void Parser::parse() 
{
    // Determine the number of threads that can run concurrently
    uint64_t numThreads = std::thread::hardware_concurrency();
    printw("Your computer supports %llu concurrent threads.\n\r", numThreads);
#ifdef USE_OMP
    printw("Using OpenMP.\n\r", numThreads);
#endif
    refresh();

    std::ifstream infile(filename);
    std::string line;
    VcdScope* currScope = nullptr;
    std::string token;

    std::regex unpackedVecRegex("^.+\\[[0-9]+\\]$");

    startMeasureTime("Parsing...");

    // read in line by line
    while (std::getline(infile, line)) 
    {
        boost::algorithm::trim(line);
        if (line.empty()) continue;

        std::istringstream ss(line);

        // parse each line token by token (space-separated)
        while (ss >> token) 
        {
            if (token.at(0) == '$') 
            {
                currState = getParseState(token);
                if (currState == PARSE_UPSCOPE) 
                {
                    currScope = currScope->parent;
                    break;
                }
            } 
            else 
            {
                switch (currState) 
                {
                    case PARSE_VERSION: 
                    {
                        version += ' ' + token;
                        break;
                    }
                    case PARSE_DATE: 
                    {
                        date += ' ' + token;
                        break;
                    }
                    case PARSE_TIMESCALE: 
                    {
                        timescale += ' ' + token;
                        break;
                    }
                    case PARSE_SCOPE: 
                    {
                        VcdScope* nextScope = new VcdScope();
                        nextScope->parent = currScope;
                        scopes.emplace_back(nextScope);
                        ss >> token;
                        nextScope->name = token;
                        if (!currScope) // top level
                        {
                            if (!topScope) 
                            {
                                topScope = nextScope;
                                currScope = nextScope;
                            } 
                            else 
                            { // visited before
                                currScope = topScope;
                            }
                            
                        }   
                        else if (!currScope->children.count(nextScope->name)) // non-top level, haven't visited before
                        {
                            currScope->children[nextScope->name] = nextScope;
                            currScope = nextScope;
                        }
                        else // top level, have visited before
                            currScope = dynamic_cast<VcdScope*>(currScope->children[nextScope->name]);
                        break;
                    }
                    case PARSE_VAR: 
                    {
                        std::string type = token;
                        ss >> token;
                        uint64_t size = stoi(token);
                        ss >> token;
                        std::string hash = token;
                        ss >> token;
                        std::string name = token;
                        ss >> token;
                        std::string dimensions = token;
                        if (!varMap.count(hash)) 
                        {
                            VcdVar* curr_var = new VcdVar();
                            curr_var->parent = currScope;
                            curr_var->size = size;
                            curr_var->hash = hash;
                            curr_var->name = name;
                            curr_var->dimensions = dimensions;
                            varMap[hash] = curr_var;
                            varHashes.push_back(hash);
                        }
                        if (std::regex_match(name, unpackedVecRegex)) 
                        { 
                            // unpacked array, extract name to use as scope
                            std::string scopeName = name.substr(0, name.find("["));
                            VcdScope* arrScope;
                            if (arrScopes.count(scopeName)) 
                            {
                                arrScope = arrScopes[scopeName];
                            }
                            else 
                            {
                                arrScope = new VcdArrScope();
                                arrScope->name = scopeName;
                                arrScope->parent = currScope;
                                arrScope->type = VcdNode::ARR_SCOPE;
                                currScope->children[scopeName] = arrScope;
                                scopes.emplace_back(arrScope);
                                arrScopes[scopeName] = arrScope;
                            }
                            varMap[hash]->parent = arrScope;
                            arrScope->children[varMap[hash]->name] = varMap[hash];
                        } 
                        else 
                        {
                            currScope->children[varMap[hash]->name] = varMap[hash];
                        }
                        break;
                    }
                    case PARSE_VALUES: 
                    {
                        if (token.at(0) == '#') 
                        {
                            currTime = stoi(token.substr(1, token.size()));
                        } 
                        else if (token.at(0) == 'b') 
                        {
                            std::string value = token;
                            ss >> token;
                            varMap[token]->vcdValues.emplace_back(
                                std::pair(currTime, value));
                        } 
                        else 
                        {
                            std::string hash = token.substr(1, token.size());
                            varMap[hash]->vcdValues.emplace_back(
                                std::pair(currTime, token.substr(0, 1)));
                        }
                        break;
                    }
                    default:
                        assert(false);
                }
            }
        }
    }
    maxTime = currTime;
    if (!version.empty()) version = version.substr(1, version.size());
    if (!timescale.empty()) timescale = timescale.substr(1, timescale.size());
    if (!date.empty()) date = date.substr(1, date.size());

    endMeasureTime("Parse Time");
    
    startMeasureTime("Processing data into value intervals...");

#ifdef USE_OMP
    constructValueIntervals();
#else
    std::list<std::thread> threads;
    uint64_t numVars = varHashes.size();
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
        topScope->name.c_str()
    );
    refresh();

}

#ifdef USE_OMP
void Parser::constructValueIntervals() 
{
    constructValueIntervals(0, varHashes.size());
}
#endif

void Parser::constructValueIntervals(uint64_t startIdx, uint64_t endIdx) 
{
#ifdef USE_OMP
#pragma omp parallel for
#endif
    for (uint64_t i = startIdx; i < endIdx; i++) 
    {
        std::string hash = varHashes[i];
        VcdVar* var = varMap[hash];
        auto it = var->vcdValues.begin();
        uint64_t prevTimestamp = it->first;
        std::string prevValue = it->second;
        ++it;
        for (; it != var->vcdValues.end(); ++it) 
        {
            var->intervalValues +=
                std::make_pair(
                    boost::icl::interval<uint64_t>::right_open(
                        prevTimestamp, it->first
                    ),
                    prevValue);
            prevTimestamp = it->first;
            prevValue = it->second;
        }
        var->intervalValues +=
            std::make_pair(
                boost::icl::interval<uint64_t>::right_open(
                    prevTimestamp, maxTime + 1
                ),       
                prevValue);
    }
}

Parser::~Parser() 
{
    // delete scopes
    for (auto& scope : scopes)
    {
        delete scope;
    }
    // delete vars
    for (auto& x : varMap) 
    {
        delete x.second;
    }
}

inline Parser::State Parser::getParseState(std::string token) 
{
    if (currState == PARSE_VALUES) return PARSE_VALUES;
    if (token == "$version") return PARSE_VERSION;
    if (token == "$date") return PARSE_DATE;
    if (token == "$timescale") return PARSE_TIMESCALE;
    if (token == "$scope") return PARSE_SCOPE;
    if (token == "$upscope") return PARSE_UPSCOPE;
    if (token == "$var") return PARSE_VAR;
    if (token == "$enddefinitions") return PARSE_ENDDEFINITIONS;
    if (token == "$end") 
    {
        if (currState == PARSE_ENDDEFINITIONS) return PARSE_VALUES;
        return PARSE_NONE;
    }
    return PARSE_ERR;
}

VcdScope* Parser::getTop() 
{
    return topScope;
}

VcdVar* Parser::getVcdVar(std::string hierarchicalName, VcdScope* scope) 
{
    size_t pos = hierarchicalName.find('.');
    std::string token;
    VcdScope* currScope = scope;
    token = hierarchicalName.substr(0, pos);
    assert(currScope->name == token);
    hierarchicalName.erase(0, pos + 1);
    while ((pos = hierarchicalName.find('.')) != std::string::npos) 
    {
        token = hierarchicalName.substr(0, pos);
        currScope = dynamic_cast<VcdScope*>(currScope->children[token]);
        hierarchicalName.erase(0, pos + 1);
    }
    return dynamic_cast<VcdVar*>(currScope->children[hierarchicalName]);
}

VcdVar* Parser::getVcdVar(std::string hierarchicalName) 
{
    return getVcdVar(hierarchicalName, topScope);
}

void Parser::startMeasureTime(const char* message) 
{
    startTime = std::chrono::high_resolution_clock::now();
    printw("%s\n\r", message);
    refresh();
}

void Parser::endMeasureTime(const char* desc) 
{
    auto elapsed = std::chrono::high_resolution_clock::now() - startTime;
    printw("%s: %llu us\n\r", desc, std::chrono::duration_cast<std::chrono::microseconds>(elapsed).count());
    refresh();
}

size_t Parser::getMaxTime() 
{
    return maxTime;
}

std::string Parser::getTimescale() 
{
    return timescale;
}
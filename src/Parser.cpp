#include <unistd.h>

#include <boost/algorithm/string.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

#include "Parser.hpp"
#include "VcdScope.hpp"
#include "VcdVar.hpp"

void Parser::parse(std::string file_name) {
    std::ifstream infile(file_name);
    std::string line;
    // uint64_t curr_time;
    VcdScope* curr_scope = nullptr;
    VcdVar* curr_var = nullptr;
    VcdScope* top_scope = nullptr;
    State curr_state = PARSE_NONE;
    std::string token;
    uint64_t token_num = 0;

    std::string version;
    std::string date;
    std::string timescale;

    while (std::getline(infile, line)) {
        boost::algorithm::trim(line);
        if (line.empty()) continue;

        std::istringstream ss(line);

        while (ss >> token) {
            if (token.at(0) == '$') {
                curr_state = getParseState(token);
                if (curr_state == PARSE_NONE) {
                    token_num = 0;
                    curr_var = nullptr;
                }
            } else {
                switch (curr_state) {
                    case PARSE_NONE:
                        break;
                    case PARSE_VERSION:
                        version += token;
                        break;
                    case PARSE_DATE:
                        date += token;
                        break;
                    case PARSE_TIMESCALE:
                        timescale += token;
                        break;
                    case PARSE_SCOPE:
                        if (token_num == 0) {
                            VcdScope* next_scope = new VcdScope();
                            next_scope->parent = curr_scope;
                            if (!curr_scope)
                                top_scope = next_scope;
                            else
                                curr_scope->children.push_back(next_scope);
                            curr_scope = next_scope;
                        } else if (token_num == 1) {
                            curr_scope->name = token;
                        } else {
                            assert(false);
                        }
                        break;
                    case PARSE_UPSCOPE:
                        curr_scope = curr_scope->parent;
                        break;
                    case PARSE_VAR:
                        if (token_num == 0) {
                            curr_var = new VcdVar();
                            curr_var->parent = curr_scope;
                            assert(curr_scope);
                            curr_scope->children.push_back(curr_var);
                        } else if (token_num == 1) {
                            curr_var->size = stoi(token);
                        } else if (token_num == 2) {
                            curr_var->hash = token;
                        } else if (token_num == 3) {
                            curr_var->name = token;
                        } else if (token_num == 4) {
                            curr_var->dimensions = token;
                        } else {
                            assert(false);
                        }
                        break;
                    case PARSE_ENDDEFINITIONS:
                        return;
                    default:
                        assert(false);
                }
                token_num++;
            }
        }

    }

    std::cout   << "Version: "      << version      << std::endl
                << "Date: "         << date         << std::endl
                << "Timescale: "    << timescale    << std::endl;

    std::cout   << "Top scope: "    << top_scope->name << std::endl;
    for (auto child : top_scope->children) {
        std::cout  << child->name << std::endl;
    }

    // delete

}

inline Parser::State Parser::getParseState(std::string token) {
    if (token == "$version")
        return PARSE_VERSION;
    if (token == "$date")
        return PARSE_DATE;
    if (token == "$timescale")
        return PARSE_TIMESCALE;
    if (token == "$scope")
        return PARSE_SCOPE;
    if (token == "$upscope")
        return PARSE_UPSCOPE;
    if (token == "$var")
        return PARSE_VAR;
    if (token == "$enddefinitions")
        return PARSE_ENDDEFINITIONS;
    if (token == "$end")
        return PARSE_NONE;
    return PARSE_ERR;
}
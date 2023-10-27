#ifndef __PARSER_H
#define __PARSER_H

#include <string>
#include <map>

#include "VcdScope.hpp"
#include "VcdVar.hpp"

class Parser {
   private:
    enum State {
        PARSE_NONE,
        PARSE_VERSION,
        PARSE_DATE,
        PARSE_TIMESCALE,
        PARSE_SCOPE,
        PARSE_UPSCOPE,
        PARSE_VAR,
        PARSE_ENDDEFINITIONS,
        PARSE_VALUES,
        PARSE_ERR
    };

    std::string filename;
    VcdScope* top_scope;
    State curr_state;
    std::map<std::string, VcdVar*> var_map;
    std::list<VcdScope*> scopes;

    inline State getParseState(std::string token);

   public:
    Parser(std::string filename)
        : filename(filename), top_scope(nullptr), curr_state(PARSE_NONE){};
    ~Parser();
    void parse();
};

#endif
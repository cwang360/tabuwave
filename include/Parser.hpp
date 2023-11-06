#ifndef __PARSER_H
#define __PARSER_H

#include <map>
#include <string>

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
    uint64_t curr_time;
    std::vector<std::string> var_hashes;
    std::map<std::string, VcdVar*> var_map;
    std::list<VcdScope*> scopes;
    std::chrono::steady_clock::time_point startTime;

    inline State getParseState(std::string token);
    void constructValueIntervals(uint64_t startIdx, uint64_t endIdx);
    void startMeasureTime();
    void endMeasureTime(std::string desc);

   public:
    Parser(std::string filename)
        : filename(filename), top_scope(nullptr), curr_state(PARSE_NONE){};
    ~Parser();
    void parse();
    VcdScope* getTop();
    VcdVar* getVcdVar(std::string hierarchicalName, VcdScope* scope);
    VcdVar* getVcdVar(std::string hierarchicalName);
};

#endif
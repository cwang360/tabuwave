#ifndef __PARSER_HPP
#define __PARSER_HPP

#include <map>
#include <string>
#include <chrono>

#include "Vcd.hpp"

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
    size_t curr_time;
    size_t maxTime;
    std::vector<std::string> var_hashes;
    std::map<std::string, VcdVar*> var_map;
    std::list<VcdScope*> scopes;
    std::chrono::high_resolution_clock::time_point startTime;

    inline State getParseState(std::string token);
    void constructValueIntervals(uint64_t startIdx, uint64_t endIdx);
#ifdef USE_OMP
    void constructValueIntervals();
#endif
    void startMeasureTime(const char* message);
    void endMeasureTime(const char* desc);

   public:
    Parser(std::string filename)
        : filename(filename), top_scope(nullptr), curr_state(PARSE_NONE){};
    ~Parser();
    void parse();
    VcdScope* getTop();
    VcdVar* getVcdVar(std::string hierarchicalName, VcdScope* scope);
    VcdVar* getVcdVar(std::string hierarchicalName);
    size_t getMaxTime();
};

#endif
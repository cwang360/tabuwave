#ifndef __PARSER_H
#define __PARSER_H

#include <string>

class Parser {
  public:
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

    static void parse(std::string file_name);

  private:
    static inline State getParseState(std::string token);
};



#endif
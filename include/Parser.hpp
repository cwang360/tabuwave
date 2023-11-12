/**
 * Author:          Cynthia Wang
 * Date created:    10/27/2023
 * Organization:    ECE 4122
 *
 * Description:
 * Header file for Parser class.
*/

#ifndef __PARSER_HPP
#define __PARSER_HPP

#include <map>
#include <string>
#include <chrono>

#include "Vcd.hpp"

/**
 * @brief Class for parsing VCD files into a traversable 
 * tree-like structure of `VcdNode`s that makes the signals
 * easier to process in Tabuwave.
 */
class Parser 
{
   private:
    /**
     * @brief enum for states of the parser. The parser is 
     * designed like a state machine for greater readability
     * and easier control flow.
     * 
     */
    enum State 
    {
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
    VcdScope* topScope;
    State currState;
    size_t currTime;
    size_t maxTime;
    std::vector<std::string> varHashes;
    std::map<std::string, VcdVar*> varMap;
    std::list<VcdScope*> scopes; // to keep track for deleting and not double-deleting
    std::map<std::string, VcdScope*> arrScopes;
    std::chrono::high_resolution_clock::time_point startTime;

    /**
     * @brief Get the parse state that the parser should transition to
     * after encountering the given token.
     * 
     * @param token (std::string) Token read from VCD file
     * @return State of parser
     */
    inline State getParseState(std::string token);

    /**
     * @brief Constructs value intervals for all `VcdVar`s in under `top_scope`.
     * startIdx and endIdx are used to distribute work when using std::thread
     * 
     * @param startIdx (uint64_t) start index of var_hashes this call is 
     * responsible for.
     * @param endIdx (uint64_t) end index of var_hashes this call is 
     * responsible for.
     */
    void constructValueIntervals(uint64_t startIdx, uint64_t endIdx);

#ifdef USE_OMP
    /**
     * @brief wrapper for constructValueIntervals(uint64_t startIdx, uint64_t endIdx)
     * that passes in 0 as startIdx and var_hashes.size() as endIdx so that one
     * call takes care of constructing value intervals for all `VcdVar`s.
     */
    void constructValueIntervals();
#endif

    /**
     * @brief Starts measuring time.
     * 
     * @param message (const char*) message to be printed in the window.
     */
    void startMeasureTime(const char* message);

    /**
     * @brief Ends measuring time and reports the time in ms elapsed since
     * the last call to `startMeasureTime`.
     * 
     * @param desc (const char*) description to be printed in the window
     * along with the total time in ms elapsed.
     */
    void endMeasureTime(const char* desc);

   public:
    /**
     * @brief Construct a new Parser object
     * 
     * @param filename (std::string) path to VCD file
     */
    Parser(std::string filename)
        : filename(filename), topScope(nullptr), currState(PARSE_NONE)
        {};
    
    /**
     * @brief Destroy the Parser object and clean up dynamically 
     * allocated memory.
     */
    ~Parser();

    /**
     * @brief parse the VCD file into a tree-like structure of `VcdNode`s
     */
    void parse();

    /**
     * @brief Get the top scope
     * 
     * @return pointer to a VcdScope object 
     */
    VcdScope* getTop();

    /**
     * @brief Get the VcdVar from the given `hierarchicalName` starting from
     * `scope`.
     * 
     * @param hierarchicalName (std::string) full hierarchical name of the 
     * signal starting from the name of `scope`.
     * @param scope (VcdScope*) pointer of VcdScope object to look under.
     * @return pointer to VcdVar object.
     */
    VcdVar* getVcdVar(std::string hierarchicalName, VcdScope* scope);
    VcdVar* getVcdVar(std::string hierarchicalName);
    size_t getMaxTime();
};

#endif
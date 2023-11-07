#ifndef __VCD_VAR_HPP
#define __VCD_VAR_HPP

#include <boost/icl/interval_map.hpp>
#include <string>

#include "VcdScope.hpp"

class VcdVar : public VcdScope {
   public:
    enum Type { WIRE, REG };

   private:
    size_t size;
    std::string dimensions;
    std::string hash;
    std::list<std::pair<uint64_t, std::string>> vcd_values;
    boost::icl::interval_map<uint64_t, std::string> interval_values;

   public:
    std::string getName();
    size_t getSize();
    std::string getValueAt(uint64_t time);
    friend class Parser;
};

#endif
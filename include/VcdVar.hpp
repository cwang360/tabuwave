#ifndef __VCD_VAR_HPP
#define __VCD_VAR_HPP

#include <string>
#include <boost/icl/interval_map.hpp>
#include "VcdScope.hpp"

class VcdVar : public VcdScope {
   public:
    enum Type { WIRE };

   private:
    size_t size;
    std::string dimensions;
    std::string hash;
    std::list<std::pair<uint64_t, std::string>> vcd_values;
    boost::icl::interval_map<uint64_t, std::string> interval_values;

   public:
    friend class Parser;
};

#endif
#ifndef __VCD_VAR_HPP
#define __VCD_VAR_HPP

#include "VcdScope.hpp"
#include <string>

class VcdVar : VcdScope {
  public:
    enum Type {
      WIRE
    };
  private:
    size_t size;
    std::string dimensions;
    std::string hash;

  public:


  friend class Parser;
};

#endif
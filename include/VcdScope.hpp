#ifndef __VCD_SCOPE_HPP
#define __VCD_SCOPE_HPP

#include <list>
#include <string>

class VcdScope {
  public:
    enum Type {
      MODULE
    };
  protected:
    std::string name;
    Type type;
    VcdScope* parent;
    std::list<VcdScope*> children;

  public:
    // VcdModule(VcdModule* parent) : parent(parent) {}

  friend class Parser;
};

#endif
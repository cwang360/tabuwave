#ifndef __VCD_SCOPE_HPP
#define __VCD_SCOPE_HPP

#include <list>
#include <string>

class VcdScope {
   public:
    enum Type { MODULE, BEGIN };

   protected:
    std::string name;
    Type type;
    VcdScope* parent;
    std::map<std::string, VcdScope*> children;

    friend class Parser;
};

#endif
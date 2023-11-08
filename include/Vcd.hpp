#ifndef __VCD_HPP
#define __VCD_HPP

#include <list>
#include <string>
#include <map>

#include <boost/icl/interval_map.hpp>


class VcdScope;

class VcdNode {
   public:
    // enum Type { MODULE, BEGIN, WIRE, REG };
    enum Type { SCOPE, VAR };

   protected:
    std::string name;
    Type type;
    VcdScope* parent;
   public:
    VcdNode(Type type) : type(type) {};
    const std::string getName();
    Type getType();
    
    friend class Parser;
};

class VcdScope : public VcdNode {
   private:
    std::map<std::string, VcdNode*> children;

   public:
    VcdScope() : VcdNode(SCOPE) {};
    const std::map<std::string, VcdNode*>& getChildren();
    
    friend class Parser;
};

class VcdVar : public VcdNode {
   private:
    size_t size;
    std::string dimensions;
    std::string hash;
    std::list<std::pair<uint64_t, std::string>> vcd_values;
    boost::icl::interval_map<uint64_t, std::string> interval_values;

   public:
    VcdVar() : VcdNode(VAR) {};
    size_t getSize();
    const std::string getValueAt(uint64_t time);
    
    friend class Parser;
};
#endif
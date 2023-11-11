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
    enum Type { SCOPE, VEC_SCOPE, VAR };

   protected:
    std::string name;
    Type type;
    VcdScope* parent;
   public:
    virtual ~VcdNode() {}
    VcdNode(Type type) : type(type) {};
    const std::string getName();
    Type getType();
    
    friend class Parser;
};

class VcdScope : public virtual VcdNode {
   protected:
    std::map<std::string, VcdNode*> children;

   public:
    VcdScope() : VcdNode(SCOPE) {};
    VcdScope(Type type) : VcdNode(type) {};
    const std::map<std::string, VcdNode*>& getChildren();
    
    friend class Parser;
};

class VcdPrimitive : virtual public VcdNode {
   public:
    virtual ~VcdPrimitive() {}
    virtual size_t getSize() = 0;
    virtual size_t getWidth() = 0;
    virtual const std::vector<std::string> getValueAt(uint64_t time, size_t size) = 0;
};

class VcdVar : public VcdPrimitive {
   private:
    size_t size;
    std::string dimensions;
    std::string hash;
    std::list<std::pair<uint64_t, std::string>> vcd_values;
    boost::icl::interval_map<uint64_t, std::string> interval_values;

   public:
    VcdVar() : VcdNode(VAR) {};
    size_t getSize();
    size_t getWidth();

    const std::vector<std::string> getValueAt(uint64_t time, size_t size);
    const std::string getRawValueAt(uint64_t time);
    
    friend class Parser;
};

class VcdVecScope : public VcdScope, public VcdPrimitive {
   public:
    VcdVecScope() : VcdNode(VEC_SCOPE) {};
    
    size_t getSize();
    size_t getWidth();
    const std::vector<std::string> getValueAt(uint64_t time, size_t size);
    
    friend class Parser;
};
#endif
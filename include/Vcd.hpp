/**
 * Author:          Cynthia Wang
 * Date created:    10/27/2023
 * Organization:    ECE 4122
 *
 * Description:
 * Header file for objects parsed from the VCD file to form
 * nodes of a tree-like structure 
*/

#ifndef __VCD_HPP
#define __VCD_HPP

#include <list>
#include <string>
#include <map>

#include <boost/icl/interval_map.hpp>


class VcdScope;

/**
 * @brief Base class for a parsed VCD node.
 */
class VcdNode 
{
   public:
    /**
     * @brief Enum for type of VcdNode
     */
    enum Type { SCOPE, ARR_SCOPE, VAR };

   protected:
    std::string name;
    Type type;
    VcdScope* parent;
   public:
    /**
     * @brief Construct a new Vcd Node object
     * 
     * @param type (Type) type of VcdNode
     */
    VcdNode(Type type) : type(type)
    {}

    /**
    * @brief Destroy the Vcd Node object
    */
    virtual ~VcdNode() 
    {}

    /**
     * @brief Get the name of the VcdNode
     * 
     * @return const std::string name
     */
    const std::string getName();

    /**
     * @brief Get the type of the VcdNode 
     * 
     * @return Type 
     */
    Type getType();
    
    friend class Parser;
};

/**
 * @brief class to represent a scope from a VCD file
 */
class VcdScope : public virtual VcdNode 
{
   protected:
    std::map<std::string, VcdNode*> children;

   public:
    /**
     * @brief Construct a new default VcdScope object
     * 
     */
    VcdScope() : VcdNode(SCOPE) 
    {}

    /**
     * @brief Construct a new Vcd Scope object with
     * more specific type
     * 
     * @param type (Type) type of VcdScope object
     */
    VcdScope(Type type) : VcdNode(type) 
    {}

    /**
     * @brief Get the children of the VcdScope
     * 
     * @return const std::map<std::string, VcdNode*>& children
     */
    const std::map<std::string, VcdNode*>& getChildren();
    
    friend class Parser;
};

/**
 * @brief Class to represent a primitive (i.e. a VcdVar or VcdVecScope which can be 
 * displayed in a Tabuwave table) from a VCD file. Acts as an interface providing
 * standard functions derived classes are expected to have; fully abstract and methods
 * must be implemented by derived classes.
 */
class VcdPrimitive : virtual public VcdNode 
{
   public:
    /**
     * @brief Destroy the VcdPrimitive object
     */
    virtual ~VcdPrimitive() 
    {}
    
    /**
     * @brief Get the size of the VcdPrimitive
     * 
     * @return size_t size
     */
    virtual size_t getSize() = 0;

    /**
     * @brief Get the Width of the VcdPrimitive (i.e. width of its column when displayed
     * in a table)
     * 
     * @return size_t width
     */
    virtual size_t getWidth() = 0;

    /**
     * @brief Get the value of the VcdPrimitive at the specified time.
     * 
     * @param time (uint64_t) time to query, unit based on VCD file's timescale.
     * @param size (size_t) size to pad vector to.
     * @return const std::vector<std::string> vector containing the primitive's 
     * value; each element represents the value at an index.
     */
    virtual const std::vector<std::string> getValueAt(uint64_t time, size_t size) = 0;
};


/**
 * @brief Class to represent a variable from a VCD file.
 */
class VcdVar : public VcdPrimitive 
{
   private:
    size_t size;
    std::string dimensions;
    std::string hash;
    std::list<std::pair<uint64_t, std::string>> vcdValues;
    boost::icl::interval_map<uint64_t, std::string> intervalValues;

   public:
    VcdVar() : VcdNode(VAR) 
    {}

    /**
     * @brief Get the size of the var.
     * 
     * @return size_t size
     */
    size_t getSize();

    /**
     * @brief Get the width of the var. (i.e. width of its column 
     * when displayed in a table)
     * 
     * @return size_t width
     */
    size_t getWidth();

    /**
     * @brief Get the value of the var at the specified time.
     * 
     * @param time (uint64_t) time to query, unit based on VCD file's timescale.
     * @param size (size_t) size to pad vector to.
     * @return const std::vector<std::string> vector containing the var's
     * value; each element represents the value at an index.
     */
    const std::vector<std::string> getValueAt(uint64_t time, size_t size);

    /**
     * @brief Get the raw value (unvectorized) of the var at the specified
     * time. 
     * 
     * @param time (uint64_t) time to query, unit based on VCD file's timescale. 
     * @return const std::string value as hex string
     */
    const std::string getRawValueAt(uint64_t time);
    
    friend class Parser;
};

/**
 * @brief Class to represent a scope for an unpacked array. Its children
 * are VcdVars, each representing an index of the unpacked array.
 */
class VcdArrScope : public VcdScope, public VcdPrimitive 
{
   public:
    /**
     * @brief Construct a new VcdArrScope object
     * 
     */
    VcdArrScope() : VcdNode(ARR_SCOPE) 
    {}
    
    /**
     * @brief Get the size of the unpacked array.
     * 
     * @return size_t size
     */
    size_t getSize();

    /**
     * @brief Get the width of the unpacked array. (i.e. width of its column 
     * when displayed in a table)
     * 
     * @return size_t width
     */
    size_t getWidth();

    /**
     * @brief Get the value of the unpacked array at the specified time.
     * 
     * @param time (uint64_t) time to query, unit based on VCD file's timescale.
     * @param size (size_t) size to pad vector to.
     * @return const std::vector<std::string> vector containing the unpacked array's
     * value; each element represents the value at an index.
     */
    const std::vector<std::string> getValueAt(uint64_t time, size_t size);
    
    friend class Parser;
};
#endif
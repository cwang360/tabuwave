#include "Vcd.hpp"

const std::string VcdNode::getName() {
    return name;
}

VcdNode::Type VcdNode::getType() {
    return type;
}

const std::map<std::string, VcdNode*>& VcdScope::getChildren() {
    return children;
}

size_t VcdVar::getSize() {
    return size;
}

const std::string VcdVar::getValueAt(uint64_t time) {
    return interval_values(time);
}
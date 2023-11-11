#include "Vcd.hpp"
#include <iomanip>
#include <cmath>

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

const std::vector<std::string> VcdVar::getValueAt(uint64_t time, size_t vec_size) {
    std::vector<std::string> value_vec;
    std::string value_str = interval_values(time);
    if (this->size > 1) {
        value_str = value_str.substr(1, value_str.size()); // remove 'b' prefix
    }
    for (size_t i = 0; i < vec_size; i++) {
        if (i < value_str.size()) {
            value_vec.emplace_back(std::string(1, value_str.at(value_str.size() - 1 - i)));
        } else if (i < this->size) {
            value_vec.emplace_back((value_str.size() == 1 && value_str.at(0) == 'x') ? "x" : "0");
        } else {
            value_vec.emplace_back(" ");
        }
    }
    return value_vec;
}

// done at runtime to take care of 'x' case
const std::string VcdVar::getRawValueAt(uint64_t time) {
    // return interval_values(time);
    std::string bin_val = interval_values(time);
    bin_val = bin_val.substr(1, bin_val.size());
    std::ostringstream stringStream;
    if (size > 1) {
        stringStream << 'h' << std::hex << std::setw(ceil(size / 4));
        if (bin_val.at(0) == 'x') {
            stringStream << std::setfill('x') << 'x';
        } else {
            uint64_t val = std::stoull(bin_val, nullptr, 2);
            stringStream << std::setfill('0') << val;
        }
    } else {
        stringStream << 'h' << bin_val;
    }
    return stringStream.str();
}

size_t VcdVar::getWidth() {
    return name.size() + 1;
}


size_t VcdVecScope::getSize() {
    return children.size();
}

size_t VcdVecScope::getWidth() {
    return std::max(name.size() + 1, (size_t) ceil(dynamic_cast<VcdVar*>(children.begin()->second)->getSize() / 4) + 2);
}

const std::vector<std::string> VcdVecScope::getValueAt(uint64_t time, size_t size) {
    std::vector<std::string> value_vec;
    int i = 0;
    for (; i < children.size(); i++) {
        std::ostringstream stringStream;
        stringStream << name << '[' << i << ']';
        value_vec.emplace_back(dynamic_cast<VcdVar*>(children[stringStream.str()])->getRawValueAt(time));
    }
    for (; i < size; i++) {
        value_vec.emplace_back(" ");
    }
    return value_vec;
}
#include "VcdVar.hpp"

std::string VcdVar::getName() {
    return name;
}

size_t VcdVar::getSize() {
    return size;
}

std::string VcdVar::getValueAt(uint64_t time) {
    return interval_values(time);
}
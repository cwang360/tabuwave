#include "VcdVar.hpp"

std::string VcdVar::getName() {
    return name;
}

std::string VcdVar::getValueAt(uint64_t time) {
    return interval_values(time);
}
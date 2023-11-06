#include <unistd.h>
#include <iostream>

#include "Parser.hpp"
#include "VcdScope.hpp"

void print_help() {
    std::cout << "tabuwave [OPTIONS]\n";
    std::cout << "  -h\t\tThis helpful output\n";
    std::cout << "  -f F\t\tPath to waveform file\n";
}

int main(int argc, char **argv) {
    // get command line arguments
    int opt;
    std::string waveform_file;
    while (-1 != (opt = getopt(argc, argv, "f:h"))) {
        switch (opt) {
            case 'f':
                waveform_file = optarg;
                break;
            case 'h':
                /* Fall through */
            default:
                print_help();
                return 0;
        }
    }

    if (waveform_file.empty()) {
        std::cerr << "No waveform file provided\n";
        print_help();
        return 1;
    }
    
    Parser parser(waveform_file);
    parser.parse();
    VcdVar* iram0 = parser.getVcdVar("mips_tb.mips.fetch.\\iram[0]");
    std::cout << iram0->getName() << std::endl;
    std::cout << iram0->getValueAt(3) << std::endl;

    VcdVar* iram1 = parser.getVcdVar("mips_tb.mips.fetch.\\iram[1]");
    std::cout << iram1->getName() << std::endl;
    std::cout << iram1->getValueAt(3) << std::endl;

    VcdVar* iram2 = parser.getVcdVar("mips_tb.mips.fetch.\\iram[2]");
    std::cout << iram2->getName() << std::endl;
    std::cout << iram2->getValueAt(3) << std::endl;

    return 0;
}
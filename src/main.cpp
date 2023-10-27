#include <unistd.h>

#include <boost/algorithm/string.hpp>
#include <fstream>
#include <iostream>
#include <sstream>

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

    return 0;
}
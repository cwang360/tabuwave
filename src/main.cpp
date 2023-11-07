#include <unistd.h>
#include <iostream>
#include <regex>
#include <iomanip>


#include "Parser.hpp"
#include "VcdScope.hpp"

#define BOLD "\033[1m"
#define PLAIN "\033[0m"
#define BLUE "\033[34m"
#define HIGHLIGHT "\033[42;97m"
#define ERROR "\033[41;97m"
#define INFO "\033[43;97m"

void print_help() {
    std::cout << "tabuwave [OPTIONS]\n";
    std::cout << "  -h\t\tThis helpful output\n";
    std::cout << "  -f F\t\tPath to waveform file\n";
}


void clear()
{
#if defined _WIN32
    system("cls");
#elif defined (__LINUX__) || defined(__gnu_linux__) || defined(__linux__)
    system("clear");
    //std::cout<< u8"\033[2J\033[1;1H"; //Using ANSI Escape Sequences 
#elif defined (__APPLE__)
    system("clear");
#endif
}

void print_table(std::list<VcdVar*> vars, uint64_t timestamp, bool lined, uint64_t highlight_idx) {
    std::list<std::string> values;
    std::vector<size_t> colWidths;
    size_t totalWidth = 8;
    std::cout << BOLD << BLUE << "t = " << timestamp << "\n\n index " << PLAIN "|";
    for (auto& var : vars) {
        values.push_back(var->getValueAt(timestamp));
        colWidths.push_back(var->getName().size() + 1);
        totalWidth += var->getName().size() + 3;
        std::cout << BOLD << BLUE << " " << var->getName() << " " << PLAIN << "|";
    }
    if (lined) std::cout << std::endl << std::string(totalWidth, '=');
    std::cout << std::endl;
    for (size_t i = 0; i < vars.front()->getSize(); i++) {
        std::string hl = "";
        if (i == highlight_idx) hl = HIGHLIGHT;
        std::cout << PLAIN << BOLD << BLUE << hl <<  std::setw(6) << i << PLAIN << hl << " |";
        size_t col = 0;
        for (auto& val : values) {
            if (i < val.size() - 1) {
                std::cout << PLAIN << hl << std::setw(colWidths.at(col)) << val.at(val.size() - 1 - i) << " |" << PLAIN;
            } else {
                std::cout << PLAIN << hl << std::setw(colWidths.at(col)) << "0" << " |" << PLAIN;
            }
            col++;
        }
        if (lined) std::cout << std::endl << std::string(totalWidth, '-');
        std::cout << std::endl;
    }
    std::cout << BOLD << BLUE << "\nt = " << timestamp << PLAIN << std::endl;
}

int main(int argc, char **argv) {
    clear();

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
    
    VcdVar* iram0 = parser.getVcdVar("mips_tb.mips.fetch.pc");
    std::cout << iram0->getName() << std::endl;
    std::cout << iram0->getValueAt(3) << std::endl;
    std::string iram0_val = iram0->getValueAt(3);

    VcdVar* iram1 = parser.getVcdVar("mips_tb.mips.fetch.next_pc");
    std::cout << iram1->getName() << std::endl;
    std::cout << iram1->getValueAt(3) << std::endl;
        std::string iram1_val = iram1->getValueAt(3);


    VcdVar* iram2 = parser.getVcdVar("mips_tb.mips.fetch.pc4");
    std::cout << iram2->getName() << std::endl;
    std::cout << iram2->getValueAt(3) << std::endl;
        std::string iram2_val = iram2->getValueAt(3);

    std::cout << INFO << "ENTER to continue\n" << PLAIN;
    getchar();
    clear();

    std::list<VcdVar*> vars;
    vars.emplace_back(iram0);
    vars.emplace_back(iram1);
    vars.emplace_back(iram2);
    bool lined = false;
    uint64_t highlightIdx = -1;
    uint64_t timestamp = 0;
    bool err = false;

    std::regex nonNegIntRegex("^[0-9]+$");

    while(1) {
        clear();
        print_table(vars, timestamp, lined, highlightIdx);
        if (err) {
            std::cout << ERROR << "Command not recognized" << PLAIN << std::endl;
            err = false;
        } 
        std::string line;
        getline(std::cin, line);
        
        if (std::regex_match(line, nonNegIntRegex)) {
            timestamp = stoull(line);
        } else if (line.size() > 1 && line.at(0) == '/' && std::regex_match(line.substr(1, line.size()), nonNegIntRegex)) {
            highlightIdx = stoull(line.substr(1, line.size()));
        } else if (line.size() == 1 && line.at(0) == 't') {
            lined = !lined;
        } else if (line.size() == 1 && line.at(0) == 'p') {
            highlightIdx--;
            // TODO: check out of bounds!
        } else if (line.size() == 1 && line.at(0) == 'n') {
            highlightIdx++;
        } else if (line.size() == 1 && line.at(0) == ',') {
            timestamp--;
            // TODO: check out of bounds!
        } else if (line.size() == 1 && line.at(0) == '.') {
            timestamp++;
        } else if (line.size() == 1 && line.at(0) == 'q') {
            break; 
        } else {
            err = true;
        }
    }

    return 0;
}
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <ncurses.h>

#include "Parser.hpp"
#include "Vcd.hpp"
#include "TuiManager.hpp"

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
    
    TuiManager tui;

    Parser parser(waveform_file);
    parser.parse();

    tui.display_bottom_line(DISPLAY_INFO, "ENTER to continue\n\r");

    refresh();
    while(getch() != '\n');
    erase(); 

    /// Signal MENU
    tui.display_menu_mode(parser.getTop());
    ///

    echo();
    
    VcdVar* iram0 = parser.getVcdVar("mips_tb.mips.fetch.pc");
    std::string iram0_val = iram0->getValueAt(3);

    VcdVar* iram1 = parser.getVcdVar("mips_tb.mips.fetch.next_pc");
    std::string iram1_val = iram1->getValueAt(3);

    VcdVar* iram2 = parser.getVcdVar("mips_tb.mips.fetch.pc4");
    std::string iram2_val = iram2->getValueAt(3);

    std::list<VcdVar*> vars;
    vars.emplace_back(iram0);
    vars.emplace_back(iram1);
    vars.emplace_back(iram2);

    tui.display_table_mode(vars);


    return 0;

}
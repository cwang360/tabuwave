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

    while (1) {
        tui.display_menu_mode(parser.getTop());
        erase();
        echo();
        tui.display_table_mode();
        erase();
        noecho();
    }

    return 0;

}
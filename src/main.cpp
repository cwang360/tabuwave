/**
 * Author:          Cynthia Wang
 * Date created:    10/27/2023
 * Organization:    ECE 4122
 *
 * Description:
 * Source file for main driver code for Tabuwave
*/

#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <ncurses.h>

#include "Parser.hpp"
#include "Vcd.hpp"
#include "TuiManager.hpp"

/**
 * @brief helper function to print usage text for Tabuwave
 * 
 */
void print_help() 
{
    std::cout << "tabuwave [OPTIONS]\n";
    std::cout << "  -h\t\tThis helpful output\n";
    std::cout << "  -f F\t\tPath to waveform file\n";
}

/**
 * @brief main/driver function for Tabuwave that parses command
 * line arguments, parses the VCD file, and launches the TUI
 * 
 * @param argc argument count
 * @param argv array of pointers to arrays of character objects
 * @return int 0 on success
 */
int main(int argc, char **argv) 
{  
    // get command line arguments
    int opt;
    std::string waveformFile;
    while (-1 != (opt = getopt(argc, argv, "f:h"))) 
    {
        switch (opt) 
        {
            case 'f':
                waveformFile = optarg;
                break;
            case 'h':
                /* Fall through */
            default:
                print_help();
                return 0;
        }
    }

    if (waveformFile.empty()) 
    {
        std::cerr << "No waveform file provided\n";
        print_help();
        return 1;
    }
    
    TuiManager tui;

    Parser parser(waveformFile);
    parser.parse();

    tui.setMaxTime(parser.getMaxTime());
    tui.setTimescale(parser.getTimescale());
    tui.displayBottomLine(DISPLAY_INFO, "ENTER to continue\n\r");

    refresh();
    while(getch() != '\n');
    erase(); 

    while (1) 
    {
        tui.displayMenuMode(parser.getTop());
        erase();
        echo();
        tui.displayTableMode();
        erase();
        noecho();
    }

    return 0;

}
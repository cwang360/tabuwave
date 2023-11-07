#include <unistd.h>
#include <iostream>
#include <regex>
#include <iomanip>
#include <ncurses.h>

#include "Parser.hpp"
#include "VcdScope.hpp"

#define COLOR_INFO 1
#define COLOR_BOLD 2
#define COLOR_ERROR 3
#define DISPLAY_INFO COLOR_PAIR(COLOR_INFO) | A_STANDOUT
#define DISPLAY_BOLD COLOR_PAIR(COLOR_BOLD) | A_BOLD
#define DISPLAY_ERROR COLOR_PAIR(COLOR_ERROR) | A_BOLD

void print_help() {
    std::cout << "tabuwave [OPTIONS]\n";
    std::cout << "  -h\t\tThis helpful output\n";
    std::cout << "  -f F\t\tPath to waveform file\n";
}

void print_table(std::list<VcdVar*> vars, uint64_t timestamp, bool lined, uint64_t highlight_idx) {
    std::list<std::string> values;
    std::vector<size_t> colWidths;
    size_t totalWidth = 8;
    attron(DISPLAY_BOLD);
    printw("t = %llu\n\n\r", timestamp);
    printw(" index |");
    for (auto& var : vars) {
        values.push_back(var->getValueAt(timestamp));
        colWidths.push_back(var->getName().size() + 1);
        totalWidth += var->getName().size() + 3;
        printw(" %s |", var->getName().c_str());
    }
    if (lined) printw("\n\r%s", std::string(totalWidth, '=').c_str());
    printw("\n\r");
    for (size_t i = 0; i < vars.front()->getSize(); i++) {
        std::string hl = "";
        attron(DISPLAY_BOLD | A_NORMAL);
        if (i == highlight_idx) attron(DISPLAY_INFO);
        printw("% 6d |", i);
        size_t col = 0;
        attrset(A_NORMAL);
        if (i == highlight_idx) attron(DISPLAY_INFO);
        for (auto& val : values) {
            if (i < val.size() - 1) {
                printw("% *c |", colWidths.at(col), val.at(val.size() - 1 - i));
            } else {
                printw("% *c |", colWidths.at(col), '0');
            }
            col++;
        }
        
        if (lined) printw("\n\r%s", std::string(totalWidth, '-').c_str());
        printw("\n\r");
    }
    attron(DISPLAY_BOLD);
    printw("t = %llu\n\n\r", timestamp);
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
    
    int c;
    initscr();    /* Start curses mode */
    start_color();			/* Start color 			*/
    raw();
    // noecho();
    keypad(stdscr, TRUE);

    init_pair(COLOR_ERROR, COLOR_RED, COLOR_WHITE);
    init_pair(COLOR_INFO, COLOR_BLUE, COLOR_WHITE);
    init_pair(COLOR_BOLD, COLOR_GREEN, COLOR_WHITE);

    std::stringstream out;

    Parser parser(waveform_file);
    parser.parse();
    
    VcdVar* iram0 = parser.getVcdVar("mips_tb.mips.fetch.pc");
    std::string iram0_val = iram0->getValueAt(3);

    VcdVar* iram1 = parser.getVcdVar("mips_tb.mips.fetch.next_pc");
    std::string iram1_val = iram1->getValueAt(3);

    VcdVar* iram2 = parser.getVcdVar("mips_tb.mips.fetch.pc4");
    std::string iram2_val = iram2->getValueAt(3);

    attron(DISPLAY_INFO);
    printw("ENTER to continue\n\r");
	
    refresh();
    
    while(getch() != '\n');

    std::list<VcdVar*> vars;
    vars.emplace_back(iram0);
    vars.emplace_back(iram1);
    vars.emplace_back(iram2);
    bool lined = false;
    uint64_t highlightIdx = -1;
    uint64_t timestamp = 0;
    bool err = false;

    std::regex nonNegIntRegex("^[0-9]+$");

    while(1)
    {
        
        c = 0;
        clear();
        print_table(vars, timestamp, lined, highlightIdx);
        if (err) {
            attron(DISPLAY_ERROR);
            printw("Command not recognized\n");
            err = false;
        } 
        refresh();

        char str[20];
        switch((c = getch())) {
        case KEY_UP:
            highlightIdx--;
            break;
        case KEY_DOWN:
            highlightIdx++;
            break;
        case KEY_LEFT:
            timestamp--;
            break;
        case KEY_RIGHT:
            timestamp++;
            break;
        case 't':
            lined = !lined;
            break;
        case ':':
            getstr(str);
            sscanf(str, "%llu", &timestamp);
            break;
        case '/':
            getstr(str);
            sscanf(str, "%llu", &highlightIdx);
            break;
        case 'Q':
            endwin();
            return 0;
        default:
            err = true;
            break;
        }

    }
    
    endwin();
    return 0;

}
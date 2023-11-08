#include "TuiManager.hpp"
#include <regex>

TuiManager::TuiManager() {
    initscr();    /* Start curses mode */
    start_color();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    init_pair(COLOR_ERROR, COLOR_RED, COLOR_WHITE);
    init_pair(COLOR_INFO, COLOR_BLUE, COLOR_WHITE);
    init_pair(COLOR_BOLD, COLOR_GREEN, COLOR_WHITE);
    
    getmaxyx(stdscr, height, width);
}

TuiManager::~TuiManager() {
    endwin();
}

void TuiManager::display_bottom_line(attr_t attr, const char* str) {
    attron(attr);
    mvprintw(height - 1, 0, str);
}

void TuiManager::clear_bottom_line() {
    move(height - 1, 0);
    clrtoeol();
}

void TuiManager::display_menu_mode(VcdScope* top) {
    move(0, 0);
    attrset(A_NORMAL);
    printw("v %s", top->getName().c_str());
    for (auto& child : top->getChildren()) {
        if (child.second->getType() == VcdNode::SCOPE) { // scope
            printw("\n\r     > %s", child.first.c_str());
        } else { // var
            printw("\n\r     %s", child.first.c_str());
        }
    }

    display_bottom_line(DISPLAY_INFO, "ENTER to continue\n\r");

    refresh();
    while(getch() != '\n');
    erase(); 

}

void TuiManager::display_table_mode(std::list<VcdVar*> vars) {
    int c;
    bool lined = false;
    uint64_t highlightIdx = -1;
    uint64_t timestamp = 0;
    bool err = false;
    std::regex nonNegIntRegex("^[0-9]+$");

    while(1)
    {
        c = 0;

        clear_bottom_line();
        if (err) {
            display_bottom_line(DISPLAY_ERROR, "Command not recognized.");
            err = false;
        } 
        refresh();
        print_table(vars, timestamp, lined, highlightIdx);
        move(height - 2, 0);

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
            return;
        default:
            err = true;
            break;
        }

    }
}

void TuiManager::print_table(std::list<VcdVar*> vars, uint64_t timestamp, bool lined, uint64_t highlight_idx) {
    WINDOW *w;
    size_t varSize = vars.front()->getSize();
    w = newpad (varSize * 2 + 3, width);
    scrollok(w, TRUE);

    std::list<std::string> values;
    std::vector<size_t> colWidths;
    size_t totalWidth = 8;
    attrset(DISPLAY_BOLD);
    move(0, 0);
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
    for (size_t i = 0; i < varSize; i++) {
        std::string hl = "";
        wattrset(w, DISPLAY_BOLD | A_NORMAL);
        if (i == highlight_idx) wattrset(w, DISPLAY_INFO);
        wprintw(w, "% 6d |", i);
        size_t col = 0;
        wattrset(w, A_NORMAL);
        if (i == highlight_idx) wattrset(w, DISPLAY_INFO);
        for (auto& val : values) {
            if (i < val.size() - 1) {
                wprintw(w, "% *c |", colWidths.at(col), val.at(val.size() - 1 - i));
            } else {
                wprintw(w, "% *c |", colWidths.at(col), '0');
            }
            col++;
        }
        
        if (lined) wprintw(w, "\n\r%s", std::string(totalWidth, '-').c_str());
        wprintw(w, "\n\r");
    }
    wattrset(w, DISPLAY_BOLD);
    wprintw(w, "t = %llu\n\n\r", timestamp);
    prefresh(w, (highlight_idx == (uint64_t)-1) ? 0 : highlight_idx, 0, 4, 0, height - 2, width);
}
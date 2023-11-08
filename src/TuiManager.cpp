#include "TuiManager.hpp"
#include <regex>

TuiManager::TuiManager() {
    initscr();    /* Start curses mode */
    start_color();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    init_pair(COLOR_ERROR, COLOR_WHITE, COLOR_RED);
    init_pair(COLOR_INFO, COLOR_BLUE, COLOR_WHITE);
    init_pair(COLOR_BOLD, COLOR_GREEN, COLOR_WHITE);
    
    getmaxyx(stdscr, height, width);
}

TuiManager::~TuiManager() {
    endwin();
}

void TuiManager::display_bottom_line(attr_t attr, const char* str, ...) {
    attrset(attr);
    va_list args;
    move(height - 1, 0);
    va_start(args, str);
    vwprintw(stdscr, str, args);
    va_end(args);
}

void TuiManager::clear_bottom_line() {
    move(height - 1, 0);
    clrtoeol();
}

void TuiManager::print_menu(WINDOW* w, VcdNode* top, unsigned int level) {
    for (auto& menuItem : visibleMenuItems) {
        if (cursorPos->node == menuItem.node) {
            wattrset(w, DISPLAY_INFO);
        } else if (selected.count((VcdVar*) menuItem.node)) {
            wattrset(w, DISPLAY_SELECTED);
        } else {
            wattrset(w, A_NORMAL);
        }
        if (menuItem.node->getType() == VcdNode::SCOPE) {
            wprintw(w, "\n\r% *c %s", 4 * menuItem.level + 1, menuItem.expanded ? 'v' : '>', menuItem.node->getName().c_str());
        } else { // var
            wprintw(w, "\n\r% *c%s", 4 * menuItem.level, ' ', menuItem.node->getName().c_str());
        }
    }
}

void TuiManager::display_menu_mode(VcdScope* top) {
    int scrollPos = 0;
    if (visibleMenuItems.empty()) {
        visibleMenuItems.emplace_back(MenuItem(top, 0));
        expand(visibleMenuItems.begin());
        cursorPos = visibleMenuItems.begin();
    }

    int c;
    bool err = false;
    while(1)
    {
        c = 0;
        WINDOW *w;
        w = newpad (300, width);
        scrollok(w, TRUE);

        clear_bottom_line();
        if (err) {
            display_bottom_line(DISPLAY_ERROR, "Command not recognized.");
            err = false;
        } else {
            display_bottom_line(DISPLAY_INFO, "%d selected. ENTER to continue.\n\r", selected.size());
        }
        refresh();
        print_menu(w, top, 0);
        refresh();
        prefresh(w, scrollPos, 0, 0, 0, height - 3, width);
        move(height - 2, 0);

        switch((c = getch())) {
        case KEY_UP:
        case 'k':
            scrollPos--;
            --cursorPos;
            break;
        case KEY_DOWN:
        case 'j':
            scrollPos++;
            ++cursorPos;
            break;
        case ' ':
            if (cursorPos->node->getType() == VcdNode::SCOPE) {
                if (cursorPos->expanded) {
                    collapse(cursorPos);
                } else {
                    expand(cursorPos);
                }
            } else {
                err = true;
            }
            break;
        case 's':
            if (cursorPos->node->getType() == VcdNode::VAR) {
                if (selected.count((VcdVar*) cursorPos->node)) {
                    selected.erase((VcdVar*) cursorPos->node);
                } else {
                    selected.insert((VcdVar*) cursorPos->node);
                }
            } else {
                err = true;
            }
            break;
        case 'C':
            selected.clear();
            break;
        case '\n':
            return;
        case 'Q':
            endwin();
            exit(0);
        default:
            err = true;
            break;
        }

    }
}

void TuiManager::display_table_mode() {
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
        // print_table(vars, timestamp, lined, highlightIdx);
        print_table(selected, timestamp, lined, highlightIdx);
        move(height - 2, 0);

        char str[20];
        switch((c = getch())) {
        case KEY_UP:
        case 'k':
            highlightIdx--;
            break;
        case KEY_DOWN:
        case 'j':
            highlightIdx++;
            break;
        case KEY_LEFT:
        case 'h':
            timestamp--;
            break;
        case KEY_RIGHT:
        case 'l':
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
            return;
        default:
            err = true;
            break;
        }

    }
}

void TuiManager::print_table(std::set<VcdVar*> vars, uint64_t timestamp, bool lined, uint64_t highlight_idx) {
    size_t maxSelectedSize = 0;
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
        if (var->getSize() > maxSelectedSize) maxSelectedSize = var->getSize();
    }
    
    WINDOW *w;
    w = newpad (maxSelectedSize * 2 + 3, width);
    scrollok(w, TRUE);

    if (lined) printw("\n\r%s", std::string(totalWidth, '=').c_str());
    printw("\n\r");
    for (size_t i = 0; i < maxSelectedSize; i++) {
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
    prefresh(w, (highlight_idx == (uint64_t)-1) ? 0 : highlight_idx, 0, 4, 0, height - 3, width);
}

void TuiManager::expand(std::list<TuiManager::MenuItem>::iterator scope_itr) {
    assert(!scope_itr->expanded);
    assert(scope_itr->node->getType() == VcdNode::SCOPE);
    auto curr_itr = std::next(scope_itr);
    for (auto& child : ((VcdScope*) scope_itr->node)->getChildren()) {
        visibleMenuItems.insert(curr_itr, MenuItem(child.second, scope_itr->level + 1));
    }
    scope_itr->expanded = true;
    scope_itr->lastChild = curr_itr;
}

void TuiManager::collapse(std::list<TuiManager::MenuItem>::iterator scope_itr) {
    assert(scope_itr->expanded);
    assert(scope_itr->node->getType() == VcdNode::SCOPE);
    visibleMenuItems.erase(std::next(scope_itr), scope_itr->lastChild);
    scope_itr->expanded = false;
}
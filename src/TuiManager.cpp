#include "TuiManager.hpp"

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
    move(height - 2, 0);
    clrtoeol();
    move(height - 1, 0);
    clrtoeol();
}

void TuiManager::print_menu(WINDOW* w, VcdNode* top, unsigned int level) {
    for (auto& menuItem : visibleMenuItems) {
        if (cursorPos->node == menuItem.node) {
            wattrset(w, DISPLAY_INFO);
        } else if (selected.count(dynamic_cast<VcdPrimitive*>(menuItem.node))) {
            wattrset(w, DISPLAY_SELECTED);
        } else {
            wattrset(w, A_NORMAL);
        }
        if (menuItem.node->getType() == VcdNode::SCOPE || menuItem.node->getType() == VcdNode::VEC_SCOPE) {
            wprintw(w, "\n\r% *c %s", 3 * menuItem.level, menuItem.expanded ? 'v' : '>', menuItem.node->getName().c_str());
        } else { // var
            wprintw(w, "\n\r% *c %s", 3 * menuItem.level, ' ', menuItem.node->getName().c_str());
        }
    }
}

void TuiManager::display_menu_mode(VcdScope* top) {
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
        print_menu(w, top, 0);
        refresh();
        int horizontalPos = std::distance(visibleMenuItems.begin(), cursorPos);
        if (horizontalPos < ((height - 3) / 2)) {
            horizontalPos = 0;
        } else {
            horizontalPos = horizontalPos - ((height - 3) / 2);
        }
        prefresh(w, horizontalPos, 0, 0, 0, height - 3, width);
        move(height - 2, 0);

        switch((c = getch())) {
        case KEY_UP:
        case 'k':
            if (cursorPos == visibleMenuItems.begin()) 
                cursorPos = std::prev(visibleMenuItems.end());
            else
                --cursorPos;
            break;
        case KEY_DOWN:
        case 'j':
            if (cursorPos == std::prev(visibleMenuItems.end()))
                cursorPos = visibleMenuItems.begin();
            else
                ++cursorPos;
            break;
        case ' ':
            if ((cursorPos->node->getType() == VcdNode::SCOPE) || (cursorPos->node->getType() == VcdNode::VEC_SCOPE)) {
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
            if ((cursorPos->node->getType() == VcdNode::VAR) || (cursorPos->node->getType() == VcdNode::VEC_SCOPE)) {
                if (selected.count(dynamic_cast<VcdPrimitive*>(cursorPos->node))) {
                    selected.erase(dynamic_cast<VcdPrimitive*>(cursorPos->node));
                } else {
                    selected.insert(dynamic_cast<VcdPrimitive*>(cursorPos->node));
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

    while(1)
    {
        c = 0;

        clear_bottom_line();
        if (err) {
            display_bottom_line(DISPLAY_ERROR, "Command not recognized.");
            err = false;
        } 
        refresh();
        print_table(selected, timestamp, lined, highlightIdx);
        move(height - 2, 0);

        char str[20];
        switch((c = getch())) {
        case KEY_UP:
        case 'k':
            highlightIdx = (highlightIdx == 0) ? maxSelectedSize - 1 : highlightIdx - 1;
            break;
        case KEY_DOWN:
        case 'j':
            highlightIdx = (highlightIdx == maxSelectedSize - 1) ? 0 : highlightIdx + 1;
            break;
        case KEY_LEFT:
        case 'h':
            if (timestamp > 0) timestamp--;
            break;
        case KEY_RIGHT:
        case 'l':
            if (timestamp < maxTime) timestamp++;
            break;
        case 't':
            lined = !lined;
            break;
        case ':':
            getstr(str);
            sscanf(str, "%llu", &timestamp);
            if (timestamp > maxTime) timestamp = maxTime;
            break;
        case '/':
            getstr(str);
            sscanf(str, "%llu", &highlightIdx);
            if (highlightIdx > maxSelectedSize - 1) highlightIdx = -1;
            break;
        case 'Q':
            return;
        default:
            err = true;
            break;
        }

    }
}

void TuiManager::print_table(std::set<VcdPrimitive*> vars, uint64_t timestamp, bool lined, uint64_t highlight_idx) {
    maxSelectedSize = 0;
    std::vector<size_t> colWidths;
    std::list<std::vector<std::string>> values;
    size_t totalWidth = 8;
    attrset(DISPLAY_BOLD);
    move(0, 0);
    printw("t = %llu\n\n\r", timestamp);
    printw(" index |");
    for (auto& var : vars) {
        size_t width = var->getWidth();
        colWidths.push_back(width);
        totalWidth += width + 2;
        printw("% *s |", width, var->getName().c_str());
        if (var->getSize() > maxSelectedSize) maxSelectedSize = var->getSize();
    }
    for (auto& var : vars) {
        values.emplace_back(var->getValueAt(timestamp, maxSelectedSize));
    }
    WINDOW *w;
    int padHeight = maxSelectedSize * 2 + 3;
    w = newpad (padHeight, width);
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
            wprintw(w, "% *s |", colWidths.at(col), val.at(i).c_str());
            col++;
        }
        
        if (lined) wprintw(w, "\n\r%s", std::string(totalWidth, '-').c_str());
        wprintw(w, "\n\r");
    }
    wattrset(w, DISPLAY_BOLD);
    wprintw(w, "t = %llu\n\n\r", timestamp);
    int horizontalPos = (highlight_idx == (uint64_t)-1) ? 0 : highlight_idx + (lined * highlight_idx + 1);
    if (horizontalPos < ((height - 3) / 2)) {
        horizontalPos = 0;
    } else {
        horizontalPos = horizontalPos - ((height - 3) / 2);
    }
    prefresh(w, horizontalPos, 0, 4, 0, height - 3, width);
}

void TuiManager::expand(std::list<TuiManager::MenuItem>::iterator scope_itr) {
    assert(!scope_itr->expanded);
    assert(scope_itr->node->getType() == VcdNode::SCOPE || scope_itr->node->getType() == VcdNode::VEC_SCOPE);
    auto curr_itr = std::next(scope_itr);
    for (auto& child : dynamic_cast<VcdScope*>(scope_itr->node)->getChildren()) {
        visibleMenuItems.insert(curr_itr, MenuItem(child.second, scope_itr->level + 1));
    }
    scope_itr->expanded = true;
    scope_itr->lastChild = curr_itr;
}

void TuiManager::collapse(std::list<TuiManager::MenuItem>::iterator scope_itr) {
    assert(scope_itr->expanded);
    assert(scope_itr->node->getType() == VcdNode::SCOPE || scope_itr->node->getType() == VcdNode::VEC_SCOPE);
    visibleMenuItems.erase(std::next(scope_itr), scope_itr->lastChild);
    scope_itr->expanded = false;
}

void TuiManager::set_max_time(size_t time) {
    maxTime = time;
}
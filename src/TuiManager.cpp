/**
 * Author:          Cynthia Wang
 * Date created:    11/08/2023
 * Organization:    ECE 4122
 *
 * Description:
 * Source file for TuiManager class and functions. See TuiManager.hpp for function descriptions.
*/

#include "TuiManager.hpp"

TuiManager::TuiManager() 
{
    // ncurses window initialization
    initscr();
    start_color();
    raw();
    noecho();
    keypad(stdscr, TRUE);

    // set up ncurses colors
    init_pair(COLOR_ERROR, COLOR_WHITE, COLOR_RED);
    init_pair(COLOR_INFO, COLOR_BLUE, COLOR_WHITE);
    init_pair(COLOR_BOLD, COLOR_BLUE, COLOR_GREEN);
    
    // initialize height and width
    getmaxyx(stdscr, height, width);
}

TuiManager::~TuiManager() 
{
    endwin();
}

void TuiManager::displayBottomLine(attr_t attr, const char* str, ...) 
{
    attrset(attr);
    va_list args;
    move(height - 1, 0);
    va_start(args, str);
    vwprintw(stdscr, str, args);
    va_end(args);
}

void TuiManager::clearBottomLine() 
{
    move(height - 2, 0);
    clrtoeol();
    move(height - 1, 0);
    clrtoeol();
}

void TuiManager::printMenu() 
{
    WINDOW *w;
    w = newpad(300, width);
    scrollok(w, TRUE);
    for (auto& menuItem : visibleMenuItems) 
    {
        // set attribute
        if (cursorPos->node == menuItem.node) 
            wattrset(w, DISPLAY_INFO);
        else if (selected.count(dynamic_cast<VcdPrimitive*>(menuItem.node))) 
            wattrset(w, DISPLAY_SELECTED);
        else 
            wattrset(w, A_NORMAL);

        // print
        if (menuItem.node->getType() == VcdNode::SCOPE || menuItem.node->getType() == VcdNode::ARR_SCOPE) 
            wprintw(w, "\n\r% *c %s", 3 * menuItem.level, menuItem.expanded ? 'v' : '>', menuItem.node->getName().c_str());
        else // var
            wprintw(w, "\n\r% *c %s", 3 * menuItem.level, ' ', menuItem.node->getName().c_str());
    }

    // determine vertical position of pad based on cursor position
    int verticalPos = std::distance(visibleMenuItems.begin(), cursorPos);

    if (verticalPos < ((height - 3) / 2))
        verticalPos = 0;
    else
        verticalPos = verticalPos - ((height - 3) / 2);

    prefresh(w, verticalPos, 0, 0, 0, height - 3, width);
}

void TuiManager::displayMenuMode(VcdScope* top) 
{
    // initialize by expanding just the top scope
    if (visibleMenuItems.empty()) 
    {
        visibleMenuItems.emplace_back(MenuItem(top, 0));
        expand(visibleMenuItems.begin());
        cursorPos = visibleMenuItems.begin();
    }

    int c;
    bool err = false;
    // control loop
    while(1)
    {
        c = 0;

        clearBottomLine();
        if (err) 
        {
            displayBottomLine(DISPLAY_ERROR, "Command not recognized.");
            err = false;
        } 
        else 
        {
            displayBottomLine(DISPLAY_INFO, "%d selected. ENTER to continue.\n\r", selected.size());
        }
        refresh();
        printMenu();
        move(height - 2, 0);
        attrset(A_NORMAL);

        switch((c = getch())) 
        {
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
            // only allow expand VcdScope or VcdArrScope
            if ((cursorPos->node->getType() == VcdNode::SCOPE) 
             || (cursorPos->node->getType() == VcdNode::ARR_SCOPE)) 
            {
                if (cursorPos->expanded)
                    collapse(cursorPos);
                else
                    expand(cursorPos);
            } 
            else 
            {
                err = true;
            }
            break;
        case 's':
            // only allow select VcdVar or VcdArrScope
            if ((cursorPos->node->getType() == VcdNode::VAR) 
             || (cursorPos->node->getType() == VcdNode::ARR_SCOPE))
            {
                if (selected.count(dynamic_cast<VcdPrimitive*>(cursorPos->node)))
                    selected.erase(dynamic_cast<VcdPrimitive*>(cursorPos->node));
                else
                    selected.insert(dynamic_cast<VcdPrimitive*>(cursorPos->node));
            } 
            else 
            {
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

void TuiManager::displayTableMode() 
{
    int c;
    lined = false;
    highlightIdx = -1;
    timestamp = 0;
    bool err = false;

    // control loop
    while(1)
    {
        c = 0;

        clearBottomLine();
        if (err) 
        {
            displayBottomLine(DISPLAY_ERROR, "Command not recognized.");
            err = false;
        } 
        refresh();
        printTable();
        move(height - 2, 0);
        attrset(A_NORMAL);

        char str[20];
        switch((c = getch())) 
        {
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

void TuiManager::printTable() 
{
    maxSelectedSize = 0;
    std::vector<size_t> colWidths;
    std::list<std::vector<std::string>> values;
    size_t totalWidth = 8;
    attrset(DISPLAY_BOLD);
    move(0, 0);
    printw("t = %llu \n\n\r", timestamp);
    printw(" index |");

    // determine widths of columns and print table header
    for (auto& var : selected) 
    {
        size_t width = var->getWidth();
        colWidths.push_back(width);
        totalWidth += width + 2;
        printw("% *s |", width, var->getName().c_str());
        if (var->getSize() > maxSelectedSize) maxSelectedSize = var->getSize();
    }

    // retrieve values for all vars
    for (auto& var : selected) {
        values.emplace_back(var->getValueAt(timestamp, maxSelectedSize));
    }

    WINDOW *w;
    int padHeight = maxSelectedSize * 2 + 3;
    w = newpad(padHeight, width);
    scrollok(w, TRUE);

    if (lined) printw("\n\r%s", std::string(totalWidth, '=').c_str());
    printw("\n\r");

    // print table rows
    for (size_t i = 0; i < maxSelectedSize; i++) 
    {
        std::string hl = "";
        wattrset(w, DISPLAY_BOLD | A_NORMAL);
        if (i == highlightIdx) wattrset(w, DISPLAY_INFO);
        wprintw(w, "% 6d |", i);
        size_t col = 0;
        wattrset(w, A_NORMAL);
        if (i == highlightIdx) wattrset(w, DISPLAY_INFO);
        for (auto& val : values) 
        {
            wprintw(w, "% *s |", colWidths.at(col), val.at(i).c_str());
            col++;
        }
        
        if (lined) wprintw(w, "\n\r%s", std::string(totalWidth, '-').c_str());
        wprintw(w, "\n\r");
    }

    // determine vertical position of pad based on cursor selection
    int verticalPos = (highlightIdx == (uint64_t)-1) ? 0 : highlightIdx + (lined * highlightIdx + 1);
    if (verticalPos < ((height - 3) / 2)) 
        verticalPos = 0;
    else 
        verticalPos = verticalPos - ((height - 3) / 2);
    prefresh(w, verticalPos, 0, 4, 0, height - 3, width);
}

void TuiManager::expand(std::list<TuiManager::MenuItem>::iterator scope_itr) 
{
    assert(!scope_itr->expanded);
    assert(scope_itr->node->getType() == VcdNode::SCOPE 
        || scope_itr->node->getType() == VcdNode::ARR_SCOPE);
    auto curr_itr = std::next(scope_itr);
    for (auto& child : dynamic_cast<VcdScope*>(scope_itr->node)->getChildren()) 
    {
        visibleMenuItems.insert(curr_itr, MenuItem(child.second, scope_itr->level + 1));
    }
    scope_itr->expanded = true;
    scope_itr->lastChild = curr_itr;
}

void TuiManager::collapse(std::list<TuiManager::MenuItem>::iterator scope_itr) 
{
    assert(scope_itr->expanded);
    assert(scope_itr->node->getType() == VcdNode::SCOPE 
        || scope_itr->node->getType() == VcdNode::ARR_SCOPE);
    visibleMenuItems.erase(std::next(scope_itr), scope_itr->lastChild);
    scope_itr->expanded = false;
}

void TuiManager::setMaxTime(uint64_t time) 
{
    maxTime = time;
}
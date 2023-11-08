#ifndef __TUI_MANAGER_HPP
#define __TUI_MANAGER_HPP

#include <ncurses.h>

#include <list>
#include <string>

#include "Vcd.hpp"

#define COLOR_INFO 1
#define COLOR_BOLD 2
#define COLOR_ERROR 3
#define DISPLAY_INFO COLOR_PAIR(COLOR_INFO) | A_STANDOUT
#define DISPLAY_BOLD COLOR_PAIR(COLOR_BOLD) | A_BOLD
#define DISPLAY_ERROR COLOR_PAIR(COLOR_ERROR) | A_BOLD
#define DISPLAY_SELECTED A_STANDOUT

class TuiManager {
   private:
    size_t height;
    size_t width;

    // for menu
    struct MenuItem {
        VcdNode* node;
        bool expanded;
        int level;
        std::list<MenuItem>::iterator lastChild;  // only valid if expanded
        MenuItem(VcdNode* node, int level)
            : node(node), expanded(false), level(level){};
    };
    std::list<MenuItem> visibleMenuItems;
    std::list<MenuItem>::iterator cursorPos;
    std::set<VcdVar*> selected;

    void print_table(std::set<VcdVar*> vars, uint64_t timestamp, bool lined,
                     uint64_t highlight_idx);
    void print_menu(WINDOW* w, VcdNode* top, unsigned int level);
    void expand(std::list<MenuItem>::iterator scope);
    void collapse(std::list<MenuItem>::iterator scope);

   public:
    TuiManager();
    ~TuiManager();
    void display_bottom_line(attr_t attr, const char* str, ...);
    void clear_bottom_line();
    void display_table_mode();
    void display_menu_mode(VcdScope* top);
};

#endif
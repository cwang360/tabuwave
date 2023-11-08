#ifndef __TUI_MANAGER_HPP
#define __TUI_MANAGER_HPP

#include <curses.h>
#include <string>
#include "Vcd.hpp"
#include <list>

#define COLOR_INFO 1
#define COLOR_BOLD 2
#define COLOR_ERROR 3
#define DISPLAY_INFO COLOR_PAIR(COLOR_INFO) | A_STANDOUT
#define DISPLAY_BOLD COLOR_PAIR(COLOR_BOLD) | A_BOLD
#define DISPLAY_ERROR COLOR_PAIR(COLOR_ERROR) | A_BOLD

class TuiManager {
   private:
    size_t height;
    size_t width;
    void print_table(std::list<VcdVar*> vars, uint64_t timestamp, bool lined, uint64_t highlight_idx);

   public:
    TuiManager();
    ~TuiManager();
    void display_bottom_line(attr_t attr, const char* str);
    void clear_bottom_line();
    void display_table_mode(std::list<VcdVar*> vars);
    void display_menu_mode(VcdScope* top);
};

#endif
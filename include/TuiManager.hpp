/**
 * Author:          Cynthia Wang
 * Date created:    11/08/2023
 * Organization:    ECE 4122
 *
 * Description:
 * Header file for TuiManager class and associated defines related to TUI.
*/

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

/**
 * @brief A wrapper class for `ncurses` functions and keeps state for 
 * Tabuwave's purposes to make it easier and more readable to manipulate the TUI.
 */
class TuiManager 
{
   private:
    size_t height;
    size_t width;

    /**
     * @brief Data structure for an item in the menu.
     */
    struct MenuItem 
    {
        VcdNode* node;
        bool expanded;
        int level;
        std::list<MenuItem>::iterator lastChild;  // only valid if expanded
        MenuItem(VcdNode* node, int level)
            : node(node), expanded(false), level(level){};
    };
    // for menu
    std::list<MenuItem> visibleMenuItems;
    std::list<MenuItem>::iterator cursorPos;
    std::set<VcdPrimitive*> selected;
    size_t maxSelectedSize;
    uint64_t maxTime;

    // for table
    uint64_t timestamp;
    bool lined;
    uint64_t highlightIdx;

    /**
     * @brief Creates a scrollable pad for the table, sets it up and 
     * fills its buffer with text based on timestamp, selected signals,
     * and other state info, then displays it in the window.
     */
    void printTable();

    /**
     * @brief Creates a scrollable pad for the menu, sets it up and 
     * fills its buffer with text based on visibleMenuItems, selected 
     * signals, and other state info, then displays it in the window.
     */
    void printMenu();

    /**
     * @brief Expands a scope menu item. All direct children of 
     * scope will be added to visibleMenuItems.
     * 
     * @param scope (std::list<MenuItem>::iterator) iterator of scope
     * from visibleMenuItems to expand.
     */
    void expand(std::list<MenuItem>::iterator scope);

    /**
     * @brief Collapses a scope menu item. All direct children of
     * scope will be removed from visibleMenuItems.
     * 
     * @param scope (std::list<MenuItem>::iterator) iterator of scope
     * from visibleMenuItems to collapse.
     */
    void collapse(std::list<MenuItem>::iterator scope);

   public:
    /**
     * @brief Construct a new TuiManager object and initializes the
     * window and colors.
     * 
     */
    TuiManager();
    
    /**
     * @brief Destroy the Tui Manager object
     * 
     */
    ~TuiManager();

    /**
     * @brief Displays text at the bottom line of the window.
     * 
     * @param attr (attr_t) Text attribute for formatting the text
     * @param str (const char*) Formatted string to display
     * @param ... Additional arguments to be substituted into `str`
     */
    void displayBottomLine(attr_t attr, const char* str, ...);

    /**
     * @brief Clears the bottom line of the window.
     */
    void clearBottomLine();

    /**
     * @brief Display the table screen and handle user input
     * to change the state of the table or quit.
     */
    void displayTableMode();

    /**
     * @brief Display the menu screen and handle user input to
     * change the state of the menu or quit.
     * 
     * @param top (VcdScope*) pointer to the top scope displayed
     * by the menu.
     */
    void displayMenuMode(VcdScope* top);

    /**
     * @brief Set the max time of the digital waveform data.
     * 
     * @param time (uint64_t) max time
     */
    void setMaxTime(uint64_t time);
};

#endif
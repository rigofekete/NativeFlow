#pragma once

#include "globals.h"


namespace HelperFunctions
{
    HDC setupCaretDC(HWND hwnd, HFONT *font);
    // Calculate total Y task position
    int CalculateTaskYPosition(const std::vector<TaskRect> *tasks, int taskIndex);
    // get text height needed after wrapping the lines
    int calculateWrappedTextHeight(HWND hwnd, const std::wstring &text, const RECT &bounds);
    LPARAM clickTaskXYPos(RECT* bounds);
};

namespace ButtonFunctions
{
    // Button click handling, called from WM_COMMAND of the main WinProc 
    void addbutton(HWND hEditBox, HWND hListBox);
    void clrbutton(HWND hEditBox, HWND hListBox);
    void delbutton(HWND hListBox);
};

namespace TaskFunctions
{
    void resetTaskAnimation(HWND hwnd, TaskRect *task, bool isKeyEscape);
    int  findTaskUnderMouse(HWND hwnd, POINT mousePoint);
    
    // void wrapEditCaret(HWND hwnd, EditCaretState *data);

    // void drawEditCaret(HWND hwnd, EditCaretState *data, WPARAM wParam);
    // void drawListCaret(HWND hwnd, ListCaretState *data, WPARAM wParam);
};

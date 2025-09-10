#include "globals.h"

////// Globals /////

bool GlobalRunning;


// Set the last key pressed so the KILLFOCUS handler of the Special Edit Control proc (TempEditSubclassProc) can check it do the operations accordingly  
WPARAM lastKeyPressed;

// static globals to store the created child window handles from WM_CREATE
HWND hListBox, hAddButton, hClrButton, hEditBox, hDeleteButton;

// global font variable to receive new created font handle
HFONT hGlobalFont;

// // Global variable to store the original EditBox and Button procedure 
WNDPROC oldEditBoxProc;
WNDPROC oldListBoxProc;
WNDPROC oldButtonProc;

win32_onscreen_buffer GlobalBackbuffer {};

ButtonBoxState CustomEditBoxState {};
ListBoxState CustomListBoxState {};
// TaskRect Task{};

int XOffset {};
int YOffset {};

animation_state GlobalAnimation {};

// Initialize members right here to get frame ready to be rendered on screen  
render_state GlobalRenderState { 
                                 true,  // ShowBackground  
                                 true,  // ShowAnimation
                                 0.5f   // AnimationAlpha
                               };


// Debug function 
// Leaving this here for documentation and examplification of how to use similar debugging techniques.  
void DebugTaskState(const TaskRect& task, const char* location) 
{
    char debug[256];
    sprintf_s(debug, "Task State at %s: fullyBlue=%d, isEditing=%d, blueCharCount=%zu\n", 
              location, task.fullyBlue, task.isEditing, task.blueCharCount);
    OutputDebugStringA(debug);
}



#include "dialog.h"
#include <globals.h>
#include <sstream>
#include <chrono>
#include "task_storage.h"


// #pragma comment(lib, "dwmapi.lib")

static WNDPROC oldButtonProc;
static WNDPROC oldStaticProc;

// TODO: Update the button hwnd names according to what they will do
// Defining the externs declared in the dialog.h on this file
HWND hwndButton1, hwndButton2, hwndButton3, hwndButton4;

/// Sublassed Buttons and Dialog Box ///
////////////////////////////////////////


// Array to store the button handle indexes
// TODO: Update the button names according to what they will do
// Assuming 3 buttons: add, Delete, Clear
ButtonBoxState dlgButtonStates[4] = {};


 // button array index initialization 
int dlgButtonIndex = -1;


// Custom Button Procedure
LRESULT CALLBACK CustomButtonProc(HWND hButton, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    // Determine which button is being processed and store them in the array, indexed accordingly
    if(hButton == hwndButton1) dlgButtonIndex = 0;
    else if(hButton == hwndButton2) dlgButtonIndex = 1;
    else if(hButton == hwndButton3) dlgButtonIndex = 2;
    else if(hButton == hwndButton4) dlgButtonIndex = 3;


    switch (uMsg)
    {
        case WM_ERASEBKGND:
            return 1; // Prevent default erasing to avoid flicker


        //////// Visual Effects for buttons //////////

        case WM_MOUSEMOVE:
        {   
            // If the mouse is within the button area and it’s not already hovered
            if (!dlgButtonStates[dlgButtonIndex].isHovered)
            {
                dlgButtonStates[dlgButtonIndex].isHovered = true;

                // Handle mouse-related events (clicks, etc.)

                // Start tracking the mouse leave event
                TRACKMOUSEEVENT tme = { sizeof(tme) };
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hButton;
                TrackMouseEvent(&tme);

                // Redraw button to provide visual feeback
                InvalidateRect(hButton, NULL, TRUE);
            }
            // IMPORTANT: return from this case instead of breaking and then call the default button class. 
            // This avoids conflicts and overlapped executions of code   
            return 0;
        }

        // Message sent when cursor leaves the client window (in this case the button window)
        case WM_MOUSELEAVE:
        {
            if(dlgButtonStates[dlgButtonIndex].isHovered)
            {
                dlgButtonStates[dlgButtonIndex].isHovered = false;

                InvalidateRect(hButton, NULL, TRUE);
                // Also invalidate dialog control area to clean the previously slightly increased hovered border  
                InvalidateRect(GetParent(hButton), NULL, TRUE);
                // Ensure repaint happens immediately 
            }
            return 0;
        }

        case WM_LBUTTONDOWN:
        {
            dlgButtonStates[dlgButtonIndex].isPressed = true;
            // Reset the hover state just in case
            dlgButtonStates[dlgButtonIndex].isHovered = false;
            
            // Set capture on the button once it is pressed
            // We need to do this to make sure the button does not remain pressed if we press and hold and then move the pointer to a different area
            // By using this together with the ReleaseCapture() in WM_LBUTTONUP, the button boolean isPressed will become false and capture will be released
            // making the button not active in this context any more 
            SetCapture(hButton);
            InvalidateRect(hButton, NULL, TRUE);

            // Ensure repaint happens immediately 
            return 0;

        }

        case WM_LBUTTONUP:
        {
            // When pressing the button, immediately reset the hover state to false to make sure we clear th states before closing the dialog control and have a clear state when we open it again 
            dlgButtonStates[dlgButtonIndex].isHovered = false; 
            
            if(dlgButtonStates[dlgButtonIndex].isPressed)
            {
                dlgButtonStates[dlgButtonIndex].isPressed = false;
                
                // Ensure mouse capture is released
                ReleaseCapture();
                // Redraw the button to remove pressed effect 
                InvalidateRect(hButton, NULL, TRUE);
                // Ensure repaint happens immediately 

                // Simulate button click
                SendMessage(GetParent(hButton), WM_COMMAND, GetDlgCtrlID(hButton), (LPARAM)hButton);
            }
            return 0;
        }



        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hButton, &ps);


            // Define and set a rounded region for the button
            RECT rect;
            GetClientRect(hButton, &rect);
            // Inflate rect negatively to reduce size
            // InflateRect(&rect, -3, -3);
            HRGN hRgn = CreateRoundRectRgn(rect.left, rect.top, rect.right, rect.bottom, 10, 10);
            // Apply the rounded region to the button window itself
            SetWindowRgn(hButton, hRgn, TRUE);

            // Draw background button colors individually
            HBRUSH hBrush {};
            HPEN hPen {};


            if(hButton == hwndButton1)
            {
                // Button background color
                hBrush = CreateSolidBrush(RGB(252,10,10));
                hPen = CreatePen(PS_SOLID, 1, RGB(252,10,10));
            }
            else if(hButton == hwndButton4)
            {
                hBrush = CreateSolidBrush(RGB(251,199,17));
                hPen = CreatePen(PS_SOLID, 1, RGB(251,199,17));
            }
            else if(hButton == hwndButton3)
            {
                // Button background color
                hBrush = CreateSolidBrush(RGB(0,122,204));
                hPen = CreatePen(PS_SOLID, 1, RGB(0,122,204));
            }
            
             // Draw background color to a different tone based on button state (hovered or pressed)
            if(dlgButtonStates[dlgButtonIndex].isHovered)
            {
                // When hovered, slightly lighten the background color
                if(hButton == hwndButton1)
                {
                    // Paint the button blue
                    hBrush = CreateSolidBrush(RGB(214,3,3));
                    hPen = CreatePen(PS_SOLID, 1, RGB(214,3,3));
                    // hPen = CreatePen(PS_SOLID, 7, RGB(214,3,3));
            
                }
                else if(hButton == hwndButton4)
                {
                    hBrush = CreateSolidBrush(RGB(252,181,16));
                    hPen = CreatePen(PS_SOLID, 1, RGB(252,181,16));
                    // hPen = CreatePen(PS_SOLID, 7, RGB(252,181,16));

                }
                else if(hButton == hwndButton3)
                {
                    hBrush = CreateSolidBrush(RGB(3,104,226));
                    hPen = CreatePen(PS_SOLID, 1, RGB(3,104,226));
                    // hPen = CreatePen(PS_SOLID, 7, RGB(3,104,226));
                }
            }
            if(dlgButtonStates[dlgButtonIndex].isPressed)
            {   
                if(hButton == hwndButton1)
                {
                    // When pressed, change the background to a custom color and/or border (I can tweak this to my liking whenever I want)
                    //
                    // Custom background while pressed 
                    hBrush = CreateSolidBrush(RGB(255,0,0));
                    hPen = CreatePen(PS_SOLID, 5, RGB(255,0,0));
                }
                if(hButton == hwndButton4)
                {
                    // When pressed, change the background to a custom color and/or border (I can tweak this to my liking whenever I want)
                    //
                    // Custom background while pressed 
                    hBrush = CreateSolidBrush(RGB(255,223,43));
                    hPen = CreatePen(PS_SOLID, 5, RGB(255,223,43));
                }
                if(hButton == hwndButton3)
                {
                    // When pressed, change the background to a custom color and/or border (I can tweak this to my liking whenever I want)
                    //
                    // Custom background while pressed 
                    hBrush = CreateSolidBrush(RGB(62,177,255));
                    hPen = CreatePen(PS_SOLID, 5, RGB(62,177,255));
                }

            }
            
            // Draw button background and border with rounded region
            HGDIOBJ oldBrush = SelectObject(hdc, hBrush);
            HGDIOBJ oldPen = SelectObject(hdc, hPen);

            // This is the function that actually paints the rounded rectangle on the window using the selected brush and pen. It takes the device context (hdc), the coordinates of the rectangle (rect.left, rect.top, rect.right, rect.bottom), and the ellipse dimensions for rounding the corners (10, 10).
            // When RoundRect is called, it uses the currently selected brush (hBrush) to fill the interior of the rounded rectangle and the currently selected pen (hPen) to draw the border of the rounded rectangle. The brush determines the fill color and pattern, while the pen determines the line color, width, and style.
            RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 10, 10);

            SelectObject(hdc, oldBrush);
            // SelectObject(hdc, oldPen);
            DeleteObject(hBrush);
            // DeleteObject(hPen);
            // Region is deleted automatically by SetWindowRgn, no need to delete hRgn here

            

            ///// TEXT //////////

            ////////////////////////////////////////////////////////////
            // Case block that deals with the custom font handle message sent from WM_CREATE
        
            HFONT hFont = (HFONT)SendMessage(hButton, WM_GETFONT, 0, 0);

            // Select the font into the DC
            HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
            ////////////////////////////////////////////////////////////

           // Draw the button text 
            ///////////////////////
            wchar_t text[256];
            // The ARRAYSIZE macro is a preprocessor macro typically defined in Windows headers that calculates the number of elements in a static array.
            // It works by using the sizeof operator to divide the total array size by the size of a single element:
            // #define ARRAYSIZE(x) (sizeof(x) / sizeof(x[0]))
            // It only works with static arrays, not dynamically allocated arrays or pointers.
            GetWindowText(hButton, text, ARRAYSIZE(text));
            // Button Text and Background colors  
            // Set text background 
            SetBkMode(hdc, TRANSPARENT);
            // White text 
            SetTextColor(hdc, RGB(255, 255, 255));
            // SetTextColor(hdc, RGB(0, 0, 0));
            DrawText(hdc, text, -1, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE );

            SelectObject(hdc, hOldFont);

            
            EndPaint(hButton, &ps);
            // Prevent default paint behaviour
            return 0;
        }
    }
    return CallWindowProc(oldButtonProc, hButton, uMsg, wParam, lParam);
}

// Custom Static Text Control Procedure
LRESULT CALLBACK CustomStaticProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    if (uMsg == WM_PAINT)
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hwnd, &ps);

        // Draw a dark background
        RECT rect;
        GetClientRect(hwnd, &rect);
        FillRect(hdc, &rect, CreateSolidBrush(RGB(40, 40, 40))); // Dark gray

        // Draw static text
        SetBkMode(hdc, TRANSPARENT);
        SetTextColor(hdc, RGB(255, 255, 255)); // White text
        
        

        int index = CustomListBoxState.clickedIndex;
       
        if(index >= 0 && !CustomListBoxState.tasks[index].status.empty())
        {
            std::wstring outputString = CustomListBoxState.tasks[index].status; 
            DrawText(hdc, outputString.c_str(), -1, &rect, DT_LEFT | DT_VCENTER);
        }
        else
        {
            // If we set the text only in the resources.rc control template then we use these 2 calls
            wchar_t text[256];
            GetWindowText(hwnd, text, ARRAYSIZE(text));
            
            DrawText(hdc, text, -1, &rect, DT_LEFT | DT_VCENTER);

            // If we want to print our own text instead with a dynamic variable, we do this:
            // wchar_t formattedText[256];

            // swprintf(formattedText, L"Deadline ends at %d", variableValue);
            
            // DrawText(hdc, formattedText, -1, &rect, DT_LEFT | DT_VCENTER);
        }

        EndPaint(hwnd, &ps);
        return 0;
    }
    return CallWindowProc(oldStaticProc, hwnd, uMsg, wParam, lParam);
}


//////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////

// Global callback for the dialog box proc //
///////////////////////////////////////
// NOTE: if this is a proc why arent we placing it along side the other procs in the WM_CREATE of the main winproc? 
// While the dialog procedure (DarkDialogProc) is a standalone callback function, it works slightly differently from the window procedures (WndProc and others) because it is designed for modal dialogs created using DialogBox.
// The key reason it is not included in the WM_CREATE handler or directly alongside your other procedures is that modal dialogs have their own internal message loop, separate from the main window’s message loop. *More documentation on why we need to do this at the bottom of the file. 
INT_PTR CALLBACK DarkDialogProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    static HBRUSH hBrushDlg = NULL;
    static HBRUSH hBrushBtn = NULL;

    switch (uMsg)
    {
        case WM_INITDIALOG:
        {
            // Apply dark mode to the dialog window
            BOOL darkMode = TRUE;
            DwmSetWindowAttribute(hwndDlg, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));

            // Create brushes for the dialog and button backgrounds
            hBrushDlg = CreateSolidBrush(RGB(40, 40, 40)); // Dark gray for dialog background
            hBrushBtn = CreateSolidBrush(RGB(60, 60, 60)); // Slightly lighter gray for buttons

            // Subclass buttons
            // These button controls are being created in templates in the resources.rc file. This abstraction replaces the usual procedure of doing CreateWindow() for each control individually, like we do in WinMain Proc. So, because the ctrls are created already, we just need to get the control handles with GetDlgItem and then subclass them for customization   
            hwndButton1 = GetDlgItem(hwndDlg, IDBUTTON1);
            hwndButton2 = GetDlgItem(hwndDlg, IDBUTTON2);
            hwndButton3 = GetDlgItem(hwndDlg, IDBUTTON3);
            hwndButton4 = GetDlgItem(hwndDlg, IDBUTTON4);

            if(hwndButton1)
            {
                oldButtonProc = (WNDPROC)SetWindowLongPtr(hwndButton1, GWLP_WNDPROC, 
                                                          (LONG_PTR)CustomButtonProc);            
            }
            if(hwndButton2)
            {
                SetWindowLongPtr(hwndButton2, GWLP_WNDPROC, (LONG_PTR)CustomButtonProc);
            }
            if(hwndButton3)
            {
                SetWindowLongPtr(hwndButton3, GWLP_WNDPROC, (LONG_PTR)CustomButtonProc);
            }
            if(hwndButton4)
            {
                SetWindowLongPtr(hwndButton4, GWLP_WNDPROC, (LONG_PTR)CustomButtonProc);
            }

            // Subclass static text
            // Same thing as the button controls 
            HWND hwndStatic = GetDlgItem(hwndDlg, IDC_STATIC);
            if (hwndStatic)
            {
                oldStaticProc = (WNDPROC)SetWindowLongPtr(hwndStatic, GWLP_WNDPROC, (LONG_PTR)CustomStaticProc);
            }

            // Center the dialog on the parent window
            HWND hwndParent = GetParent(hwndDlg);
            RECT rcParent, rcDlg;
            GetWindowRect(hwndParent, &rcParent);
            GetWindowRect(hwndDlg, &rcDlg);

            int posX = rcParent.left + (rcParent.right - rcParent.left - (rcDlg.right - rcDlg.left)) / 2;
            int posY = rcParent.top + (rcParent.bottom - rcParent.top - (rcDlg.bottom - rcDlg.top)) / 2;

            SetWindowPos(hwndDlg, NULL, posX, posY, 0, 0, SWP_NOZORDER | SWP_NOSIZE);
            return TRUE;
        }

        // Background color control customization for the Dialog box (this is not able to handle static text controls or any other child controls, like buttons. This is the reason why we need to subclass those controls in order to customize them)
        case WM_CTLCOLORDLG:
        {
            // Set dialog background to transparent to avoid artifacts and default bkgd color and return the hBrushDlg which is already set to the background color by this time in the WM_INITDIALOG case 
            HDC hdc = (HDC)wParam;
            SetBkMode(hdc, TRANSPARENT);
            return (INT_PTR)hBrushDlg;
        }

        case WM_COMMAND:
        {
            switch (LOWORD(wParam))
            {
                // Urgent button, 48 hours
                case IDBUTTON1:
                {
                    int selectedIndex = CustomListBoxState.clickedIndex;

                    if(selectedIndex >= 0 && selectedIndex < CustomListBoxState.tasks.size())
                    {
                        TaskRect &task = CustomListBoxState.tasks[selectedIndex];

                        // Set deadline to 2 days from now
                        auto now = std::chrono::system_clock::now();
                        // auto stands for:
                        // std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
                        task.deadline = now + std::chrono::hours(48);
                        // seconds setter for debugging purposes   
                        // task.deadline = now + std::chrono::seconds(1);  

                        task.data.deadline = task.deadline;

                        try
                        {
                            TaskStorage::saveTasksToFile(CustomListBoxState.tasks);
                        }
                        catch (const std::exception &e) 
                        {
                            std::cerr << "Failed to save tasks: " << e.what() << '\n';
                        }

                        // Assign a unique timer ID and start the timer
                        if(task.deadlineTimerID)
                        {
                            // Stop any previous timer 
                            KillTimer(hwndDlg, task.deadlineTimerID);
                        }

                        // Unique ID (by adding 300 to the index value)
                        task.deadlineTimerID = selectedIndex + 300;
                        task.data.activeDeadline = true;
                        // reset expired member in case we are setting a new deadline to an already expired task  
                        task.expired = false;
                        SetTimer(GetParent(hwndDlg), task.deadlineTimerID, 1000, NULL);
                    }

                    EndDialog(hwndDlg, IDBUTTON1);
                    return TRUE;
                }
                // This Flag and handler is strictly to close the dialog box since the value 2 seems to be reserved by default to the close handler/button 
                case IDBUTTON2:
                {   
                    EndDialog(hwndDlg, IDBUTTON2);
                    return TRUE;
                }
                // Sooon button, set deadline to 96 hours
                case IDBUTTON4:
                {   
                    int selectedIndex = CustomListBoxState.clickedIndex;

                    if(selectedIndex >= 0 && selectedIndex < CustomListBoxState.tasks.size())
                    {
                        TaskRect &task = CustomListBoxState.tasks[selectedIndex];

                        // Set deadline to 2 days from now
                        auto now = std::chrono::system_clock::now();
                        // auto stands for:
                        // std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
                        task.deadline = now + std::chrono::hours(96);
                        // seconds setter for debugging purposes   
                        // task.deadline = now + std::chrono::seconds(1);  

                        task.data.deadline = task.deadline;

                        try
                        {
                            TaskStorage::saveTasksToFile(CustomListBoxState.tasks);
                        }
                        catch (const std::exception &e) 
                        {
                            std::cerr << "Failed to save tasks: " << e.what() << '\n';
                        }

                        // Assign a unique timer ID and start the timer
                        if(task.deadlineTimerID)
                        {
                            // Stop any previous timer 
                            KillTimer(hwndDlg, task.deadlineTimerID);
                        }

                        // Unique ID (by adding 300 to the index value)
                        task.deadlineTimerID = selectedIndex + 300;
                        task.data.activeDeadline = true;
                        // reset expired member in case we are setting a new deadline to an already expired task  
                        task.expired = false;
                        SetTimer(GetParent(hwndDlg), task.deadlineTimerID, 1000, NULL);
                    }

                    EndDialog(hwndDlg, IDBUTTON4);
                    return TRUE;
                }
                // Free button, basically reset deadlines 
                case IDBUTTON3:
                {
                    int selectedIndex = CustomListBoxState.clickedIndex;

                    if(selectedIndex >= 0 && selectedIndex < CustomListBoxState.tasks.size())
                    {
                        TaskRect &task = CustomListBoxState.tasks[selectedIndex];
                        

                        // Properly reset all the deadline related members of the task. 
                        task.status = L"No deadline is set";
                        // This is the proper way to reset the time_point variable type 
                        task.deadline = std::chrono::system_clock::time_point{};
                        task.data.deadline = task.deadline;
                        task.data.activeDeadline = false;
                        // Both the task rect and task data members get reset 
                        task.data.expired = task.expired = false;

                        try
                        {
                            TaskStorage::saveTasksToFile(CustomListBoxState.tasks);
                        }
                        catch (const std::exception &e) 
                        {
                            std::cerr << "Failed to save tasks: " << e.what() << '\n';
                        }

                     
                        // Kill any active timer 
                        if(task.deadlineTimerID)
                        {
                            KillTimer(hwndDlg, task.deadlineTimerID);
                        }
                        
                        // reset timer ID
                        task.deadlineTimerID = 0;

                    }

                    EndDialog(hwndDlg, IDBUTTON3);
                    return TRUE;
                }
            }
            break;
        }
        
        // 
        // case WM_CLOSE:
        //     EndDialog(hwndDlg, IDBUTTON2);
        //     return TRUE;

        case WM_DESTROY:
        {
            // Clean up brushes
            if (hBrushDlg)
            {
                DeleteObject(hBrushDlg);
                hBrushDlg = NULL;
            }
            if (hBrushBtn)
            {
                DeleteObject(hBrushBtn);
                hBrushBtn = NULL;
            }
            if (oldButtonProc)
            {
                SetWindowLongPtr(GetDlgItem(hwndDlg, IDBUTTON1), GWLP_WNDPROC, (LONG_PTR)oldButtonProc);
                SetWindowLongPtr(GetDlgItem(hwndDlg, IDBUTTON2), GWLP_WNDPROC, (LONG_PTR)oldButtonProc);
                SetWindowLongPtr(GetDlgItem(hwndDlg, IDBUTTON3), GWLP_WNDPROC, (LONG_PTR)oldButtonProc);
                SetWindowLongPtr(GetDlgItem(hwndDlg, IDBUTTON4), GWLP_WNDPROC, (LONG_PTR)oldButtonProc);
            }
            if (oldStaticProc)
            {
                SetWindowLongPtr(GetDlgItem(hwndDlg, IDC_STATIC), GWLP_WNDPROC, (LONG_PTR)oldStaticProc);
            }
            return TRUE;
        }
    }
    return FALSE;
}

// Function to create and show the dark-themed dialog
void ShowDarkDialog(HWND hwndParent)
{
    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_DARK_DIALOG), hwndParent, DarkDialogProc);
}

void ShowCountDialog(HWND hwndParent)
{
    DialogBox(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_COUNT_DIALOG), hwndParent, DarkDialogProc);
}



//////////////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////////////

// *Why Not Place DarkDialogProc in WM_CREATE?

// Modal Dialog Lifecycle:

// A modal dialog like the one created with DialogBox blocks interaction with its parent window until it is dismissed (e.g., by clicking "OK" or "Cancel"). It doesn't operate within the same event handling flow as the main window.
// Modal dialogs require a specific callback (DialogProc) to process their messages.
// Separation of Concerns:

// The WM_CREATE in your main window’s WndProc is meant for initializing controls and properties of the main window.
// Dialog-related code, being self-contained, should live outside the WndProc for clarity and maintainability.
// Dynamic Display:

// Dialogs are typically invoked dynamically in response to user actions (e.g., clicking a menu item or button). They are not part of the main window's static controls initialized during WM_CREATE.

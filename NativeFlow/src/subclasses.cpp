#include "subclasses.h"



// We need this to add a special independent Edit Control specifically for the List Box task, in order to handle the default behaviour for caret, text editing, wraps so se dont have to manually do these complex and error prone operations.
LRESULT CALLBACK TempEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, 
                                      LPARAM lParam, UINT_PTR uIdSubclass, 
                                      DWORD_PTR dwRefData)
{
    TaskRect  *task = (TaskRect*)dwRefData;

    switch(msg)
    {   
        case WM_ERASEBKGND:
            return 1; // Prevent default background erase

        case WM_KEYDOWN:
        {
            if(wParam == VK_ESCAPE || wParam == VK_RETURN)
            {
                if(task)
                {   
                    // Set the last key pressed so the KILLFOCUS handler can check it do the operations accordingly  
                    lastKeyPressed = wParam;

                    if(wParam == VK_RETURN)
                    {
                        int len = GetWindowTextLength(hwnd); 
                        std::vector<wchar_t> buffer(len + 1);
                        GetWindowText(hwnd, buffer.data(), len + 1);  
                        task->text = buffer.data();
                        task->data.text = task->text;

                        TaskRect &taskEdit = *task;

                        // try
                        // {
                        //     // There should only ever be ONE network manager in your application (you don't want multiple network connections competing)
                        //     // We need this one instance to be accessible from anywhere in the code
                        //     // We need to ensure it's created only when first needed
                        //     NetworkManager::getInstance().syncTaskAdded(taskEdit);
                        // }
                        // catch(const std::exception &e)
                        // {
                        //     // For now, just log the error and continue
                        //     std::cerr << "Failed to sync task: " << e.what() << std::endl;
                        //     // If network sync fails, add to pending sync
                        //     TaskStorage::addPendingSync(taskEdit);
                        // }

                        // Save updated local list 
                        try
                        {
                            TaskStorage::saveTasksToFile(CustomListBoxState.tasks);
                        }
                        catch(const std::exception& e)
                        {
                            std::cerr << "Failed to save task locally: " << e.what() << '\n';
                        }
                    }

                    // If only Escape or Return was pressed, do the following 
                    TaskFunctions::resetTaskAnimation(hwnd, task, false);
                   
                    // Uncomment this to force a click on a task after pressing escape from the editing mode
                    SendMessage(hListBox, WM_LBUTTONDOWN, MK_LBUTTON, 0);

                    // We need to invalidate the listbox in order to repaint the updated positions in case the edited task rects height changed
                    InvalidateRect(hListBox, NULL, TRUE);
                    UpdateWindow(hwnd);

                    // // Set focus to the default Edit Box to keep things tidy
                    // SetFocus(hEditBox);

                    break;
                }
            }
            else if (wParam == VK_BACK)
            {
                DefSubclassProc(hwnd, msg, wParam, lParam);
                RECT testRect;
                GetClientRect(hwnd, &testRect);
                testRect.bottom += 20;
                // InvalidateRect(hwnd, &testRect, TRUE);
                
                 RedrawWindow(hwnd, &testRect, NULL, 
                RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
            }
            break;
        }

        case WM_MOUSEWHEEL:
        {
            // Block all mouse wheel scroll events while in edit mode
            return 0;
        }

        case WM_PAINT:
        {   
            // paint the hover borders with pink color to signal we are in text edit mode  
            if(task->isEditing)
            {
                // TODO: Not sure if we'll ever need this but leaving it here just in case...
                // // First let the default proc handle the edit control background and text
                LRESULT result = DefSubclassProc(hwnd, msg, wParam, lParam);

                HDC hdc = GetDC(GetParent(hwnd));
                
                RECT mappedBounds = task->bounds;
                // Map the task bounds from the ListBox client area to the parent window client area to draw the borders in the same exact place, with converted coordinates relative to the main window
                //
                // We do this because the task.bounds we are using are coming from the list box, therefore, its coordinates relative to the List Box client. We need to convert them to the man window, which is the parent client area of the special Edit Control   
                MapWindowPoints(hListBox, GetParent(hwnd), (LPPOINT)&mappedBounds, 2);

                // // Get current edit text height for proper bounds
                // RECT editRect;
                // GetClientRect(hwnd, &editRect);
                // mappedBounds.bottom = mappedBounds.top + (editRect.bottom - editRect.top) + (2 * taskXPadding) + 4;


                HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
                HPEN pinkPen = CreatePen(PS_SOLID, 2, RGB(200, 29, 141));
                HPEN oldPen = (HPEN)SelectObject(hdc, pinkPen);
                HPEN oldBrush = (HPEN)SelectObject(hdc, nullBrush);

                InflateRect(&mappedBounds, -1, -1);
                RoundRect(hdc, mappedBounds.left, mappedBounds.top,
                          mappedBounds.right, mappedBounds.bottom, 
                          taskXPadding, taskXPadding);
                
                SelectObject(hdc, oldPen);
                SelectObject(hdc, oldBrush);
                DeleteObject(pinkPen);
                ReleaseDC(GetParent(hwnd), hdc);

                // InvalidateRect(GetParent(hwnd), NULL, TRUE);

                return result;
            }
            break;
        }

        case WM_CHAR:
        {   
            if(wParam == VK_BACK)
            {
                // Disable repaints temporarily
                SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);
                
                // Let default proc handle deletion
                CallWindowProc(DefSubclassProc, hwnd, msg, wParam, lParam);
                
                // Get edit control rect and convert to parent coordinates
                RECT rcEdit;
                GetClientRect(hwnd, &rcEdit);
                RECT rcParent = rcEdit;
                MapWindowPoints(hwnd, GetParent(hwnd), (LPPOINT)&rcParent, 2);
                
                // TODO: I think we don't need this, but keep an eye on it. 
                // // Inflate the rect slightly to ensure we cover the entire affected area
                // InflateRect(&rcParent, 2, 2);
                
                // Paint the parent area first with grey background
                HDC hdcParent = GetDC(GetParent(hwnd));
                HBRUSH greyBrush = CreateSolidBrush(RGB(40, 40, 40));
                FillRect(hdcParent, &rcParent, greyBrush);
                ReleaseDC(GetParent(hwnd), hdcParent);
                         
                // Re-enable drawing and refresh
                SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
                RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
                
                return 0;
            }
            break;
        }

        case WM_SETFOCUS:
        {
            DefSubclassProc(hwnd, msg, wParam, lParam);

            // Create custom sized squared block caret 
            CreateCaret(hwnd, NULL, 10, 15);
            ShowCaret(hwnd);

            break;
        }


        case WM_KILLFOCUS:
        {   
            // When we are in Editing state, only to change focus when Escape or Return keys are pressed, otherwise send back the focus to the Edit Control Handle. 
            if(lastKeyPressed != VK_ESCAPE || lastKeyPressed != VK_RETURN)
            {
                SetFocus(hwnd);  // Force focus back to edit control
                return 0;
            }

            // Always reset flag after the above check 
            lastKeyPressed = 0;

            // Let the default procedure handle what is needed
            DefSubclassProc(hwnd, msg, wParam, lParam);

            // destroy our custom caret 
            HideCaret(hwnd);
            DestroyCaret();

            break;
            // return 0;
        }



        case WM_DESTROY:
        {
            RemoveWindowSubclass(hwnd, TempEditSubclassProc, uIdSubclass);
            break;
        } 
    }

    return DefSubclassProc(hwnd, msg, wParam, lParam);
}


/// SUBCLASSING BUTTON CONTROLS //////////

// Array to store the button handle indexes
// Assuming 3 buttons: Add, Delete, Clear
ButtonBoxState ButtonStates[3] = {};


 // button array index initialization 
int buttonIndex = -1;


// // Subclass procedure for button controls 
LRESULT CALLBACK ButtonProc(HWND hButton, UINT msg, WPARAM wParam, LPARAM lParam)
{
    // // button array index initialization 
    // int buttonIndex = -1;
    
    // Determine which button is being processed and store them in the array, indexed accordingly
    if(hButton == hAddButton) buttonIndex = 0;
    else if(hButton == hDeleteButton) buttonIndex = 1;
    else if(hButton == hClrButton) buttonIndex = 2;


    switch(msg)
    {
        //////// Visual Effects for buttons //////////

        case WM_MOUSEMOVE:
        {
            // If the mouse is within the button area and it’s not already hovered
            if (!ButtonStates[buttonIndex].isHovered)
            {
                ButtonStates[buttonIndex].isHovered = true;

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
            if(ButtonStates[buttonIndex].isHovered)
            {
                ButtonStates[buttonIndex].isHovered = false;

                InvalidateRect(hButton, NULL, TRUE);
                // Ensure repaint happens immediately 
            }
            return 0;
        }

        case WM_LBUTTONDOWN:
        {
            ButtonStates[buttonIndex].isPressed = true;
            // Reset the hover state just in case
            ButtonStates[buttonIndex].isHovered = false;
            
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
            
            if(ButtonStates[buttonIndex].isPressed)
            {
                ButtonStates[buttonIndex].isPressed = false;
                
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

        // This switch case msg handling deals with the problem of the flickering child round shaped buttons I was having 
        
        // Default Background Erase: Windows typically handles WM_ERASEBKGND to fill the background of a control with a solid color 
        // (often white or the system default color) before WM_PAINT is called. 
        // This process is designed to ensure that no leftover graphical data from previous frames appears, which would be problematic in most static UI scenarios

        // Bypassing Erase: Returning 1 tells Windows that you’re handling the background painting, 
        // so it doesn’t need to perform its default erase operation. This way, only your painting logic in WM_PAINT executes, 
        // avoiding the brief display of the default background color and reducing flicker.
        case WM_ERASEBKGND:
            return TRUE; // Prevent flickering
            // Return TRUE or 1: This indicates that the background erase operation has been handled, so Windows does not need to perform any default background erasure. 
            // By returning TRUE or 1, you effectively prevent Windows from clearing the control’s background with the default color, avoiding any flickering that would occur if it did.

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

            if(hButton == hAddButton)
            {
                // Button background color
                hBrush = CreateSolidBrush(RGB(0,122,204));
                // Button border color
                // hPen = CreatePen(PS_SOLID, 2, RGB(0,122,204));
            }
            else if(hButton == hDeleteButton)
            {
                hBrush = CreateSolidBrush(RGB(5,190,163));
                // Button border color
                // hPen = CreatePen(PS_SOLID, 2, RGB(0,122,204));
            }
                else if(hButton == hClrButton)
            {
                // Button background color
                hBrush = CreateSolidBrush(RGB(198,0,0));
                // Button border color
                // hPen = CreatePen(PS_SOLID, 2, RGB(0,122,204));
            }

            // Draw background color to a different tone based on button state (hovered or pressed)
            if(ButtonStates[buttonIndex].isHovered)
            {
                // When hovered, slightly lighten the background color
                // hBrush = CreateSolidBrush(RGB(50,50,50));
                if(hButton == hAddButton)
                {
                    // Paint the button blue
                    hBrush = CreateSolidBrush(RGB(40,169,255));
                    // Also the border
                    // hPen = CreatePen(PS_SOLID, 2, RGB(0,122,204));
                }
                else if(hButton == hDeleteButton)
                {
                    // hBrush = CreateSolidBrush(RGB(38,249,217)); 
                    hBrush = CreateSolidBrush(RGB(6,223,190));
                    // // Blue border when hovered 
                    // hPen = CreatePen(PS_SOLID, 2, RGB(0,122,204));
                }
                else if(hButton == hClrButton)
                {
                    //  MY WORKING CODE
                    // Paint yellow button  
                    hBrush = CreateSolidBrush(RGB(255,0,0));
                    // // Blue border when hovered 
                    // hPen = CreatePen(PS_SOLID, 2, RGB(0,122,204));
                }
            }
            if(ButtonStates[buttonIndex].isPressed)
            {   
                if(hButton == hAddButton)
                {
                    // When pressed, change the background to a custom color and/or border (I can tweak this to my liking whenever I want)
                    //
                    // Custom background while pressed 
                    hBrush = CreateSolidBrush(RGB(0,102,215));
                    // Custom border color when pressed 
                    // hPen = CreatePen(PS_SOLID, 2, RGB(0,122,204));
                }
                if(hButton == hDeleteButton)
                {
                    // When pressed, change the background to a custom color and/or border (I can tweak this to my liking whenever I want)
                    //
                    // Custom background while pressed 
                    hBrush = CreateSolidBrush(RGB(0,200,110));
                    // hBrush = CreateSolidBrush(RGB(84,222,142));
                    // Custom border color when pressed 
                    // hPen = CreatePen(PS_SOLID, 2, RGB(0,122,204));
                }
                if(hButton == hClrButton)
                {
                    // When pressed, change the background to a custom color and/or border (I can tweak this to my liking whenever I want)
                    //
                    // Custom background while pressed 
                    hBrush = CreateSolidBrush(RGB(254,99,27));
                    // hBrush = CreateSolidBrush(RGB(254,67,7));
                    // Custom border color when pressed 
                    // hPen = CreatePen(PS_SOLID, 2, RGB(0,122,204));
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
            const int textLength = GetWindowTextLength(hButton);
            wchar_t *text = new wchar_t[textLength + 1];
            GetWindowText(hButton, text, textLength +1);
            // Button Text and Background colors  
            // Set text background 
            SetBkMode(hdc, TRANSPARENT);
            // White text 
            SetTextColor(hdc, RGB(255, 255, 255));
            // SetTextColor(hdc, RGB(0, 0, 0));
            DrawText(hdc, text, textLength, &rect, DT_CENTER | DT_VCENTER | DT_SINGLELINE );

            delete[] text;
            SelectObject(hdc, hOldFont);
            

            EndPaint(hButton, &ps);
            // Prevent default paint behaviour
            return 0;
        }
    }
    // IMPORTANT: At the end of the function, after the switch case, call our created Button procedure for unhandled messages   
    return CallWindowProc(oldButtonProc, hButton, msg, wParam, lParam); 
}

/////////////////////////
/// Edit Box Subclass ///
/////////////////////////


LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {   
        case WM_ERASEBKGND:
            return 1; // Prevent flickering

        case WM_GETDLGCODE:
        {
            // Tell Windows we want to handle ALL keyboard input, including TAB
            return DLGC_WANTALLKEYS;
        }

        case WM_KEYDOWN:
        {
            if(wParam == VK_RETURN)
            {
                if(!CustomEditBoxState.text.empty())
                {
                    SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)CustomEditBoxState.text.c_str());

                    // Clear the edit control text
                    // To effectively "clean" the text painted on the edit box, we need to simply send an empty text to it with the SetWindowText() call. Default behaviour will then take care of the caret, etc  
                    SetWindowText(hwnd, L"");
                    // Clear the actual text data in the struct  
                    CustomEditBoxState.text.clear();

                    // Reset edit box to default height
                    RECT currentRect;
                    GetWindowRect(hwnd, &currentRect);
                    int currentWidth = currentRect.right - currentRect.left;
                    SetWindowPos(hwnd, NULL,
                                0, 0,
                                currentWidth,
                                editBoxHeight,
                                SWP_NOMOVE | SWP_NOZORDER);

                    InvalidateRect(hwnd, NULL, TRUE);
                    UpdateWindow(hwnd);
                }
                // Preent default behaviour
                return 0;
            }
            
            // Add selection handling
            // If BOTH shift and arrows/home/end are getting pressed, redraw the whole edit box client area to always show the whole text, even if wrapped in multiple lines.  
            if (GetKeyState(VK_SHIFT) < 0 && 
                (wParam == VK_LEFT || wParam == VK_RIGHT || 
                 wParam == VK_UP || wParam == VK_DOWN || 
                 wParam == VK_HOME || wParam == VK_END))
            {
                LRESULT result = CallWindowProc(oldEditBoxProc, hwnd, msg, wParam, lParam);
                RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
                return result;
            }
            // If ONLY the arrows/home/end are getting pressed, redraw the whole edit box client area to always show the whole text, even if wrapped in multiple lines. 
            else if(wParam == VK_LEFT || wParam == VK_RIGHT || 
                    wParam == VK_UP || wParam == VK_DOWN || 
                    wParam == VK_HOME || wParam == VK_END)
            {
                LRESULT result = CallWindowProc(oldEditBoxProc, hwnd, msg, wParam, lParam);
                RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
                return result;
            }
            break;
        }

        // We intercept this message to adjust the centering of the current text within the current resized edit box rect (which was done through the WM_COMMAND of the Main Winproc)
        case WM_SIZE:
        {
            LRESULT result = CallWindowProc(oldEditBoxProc, hwnd, msg, wParam, lParam);
            
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            
            // Only apply horizontal and vertical padding
            RECT formatRect = clientRect;
            formatRect.top += taskYSpacing; 
            formatRect.left += taskXPadding;
            formatRect.right -= taskXPadding;
            
            // Let edit control use the formatRect with the appropriate padding to center the text 
            // EM_SETRECT is a special message specifically for edit controls that sets the formatting rectangle - this is the area within the edit control where text can be displayed
            // Without EM_SETRECT, the edit control would use its entire client area for text. By setting this rect, you're creating the visual padding that centers the text vertically.
            SendMessage(hwnd, EM_SETRECT, 0, (LPARAM)&formatRect);

            InvalidateRect(hwnd, NULL, TRUE);
            
            return result;
        }

        case WM_CHAR:
        {   
            // Prevent Enter from being processed as a character which would wrap to a new line
            if(wParam == VK_RETURN)
            {
                return 0;
            }
            else if (wParam == VK_BACK)
            {
                // Disable repaints temporarily
                SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);
                
                // Let default proc handle the deletion
                CallWindowProc(oldEditBoxProc, hwnd, msg, wParam, lParam);
                
                 // Get updated text after backspace
                int len = GetWindowTextLength(hwnd);
                if(len > 0) 
                {
                    std::vector<wchar_t> buffer(len + 1);
                    GetWindowText(hwnd, buffer.data(), len + 1);
                    CustomEditBoxState.text = buffer.data();
                } 
                else 
                {
                    CustomEditBoxState.text.clear();
                }

                // We need to do this to properly to erase remnants of the blue paint from the border hover, after the edit box rect height increases/decreasing due to the line wraps/unwraps.
                // We map coordinates from the actual Windows screen to the edit rect parent client area (main background), and delete this specific region only (fine tuned with the help of the InflateRect adjustments). We need to do this this way since the remnants are going to be left as artifacts in the main client area (e.g. when deleting edit box and reducing its height after line wrap, the previous area where the remnants where no belong to the main client area and not the edit box client area)
                RECT mappedParentRect; 
                GetWindowRect(hwnd, &mappedParentRect);
                MapWindowPoints(HWND_DESKTOP, GetParent(hwnd), (LPPOINT)&mappedParentRect, 2);
                InflateRect(&mappedParentRect, 10, 10);

                // Single redraw operation
                InvalidateRect(GetParent(hwnd), &mappedParentRect, FALSE);

                // Re-enable and update
                SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);

                // We set the bErase flag from the call to FALSE because The RoundRect call in the WM_PAINT with the solid brush completely covers the previous content. It's not transparent - it's opaque. we dont need this one to repaint so you're not getting any overlapping of old and new content.
                InvalidateRect(hwnd, NULL, FALSE);

                // Return 0 to inform main we dealt with this message and prevent any additional default processing that might cause double painting 
                return 0;
            }

            // TODO:Study the new disable/enable paint approach since it got rid of the text flickering in this typing condition 
            if(wParam >= 32 || VK_BACK)
            {
                // Disable window updates temporarily
                SendMessage(hwnd, WM_SETREDRAW, FALSE, 0);
                
                CallWindowProc(oldEditBoxProc, hwnd, msg, wParam, lParam);
                
                int len = GetWindowTextLength(hwnd);
                if(len > 0) {
                    std::vector<wchar_t> buffer(len + 1);
                    GetWindowText(hwnd, buffer.data(), len + 1);
                    CustomEditBoxState.text = buffer.data();
                }

                // Set up to clean the right margin remnants of the new caret square block (when wrapping to a new line)
                // for more details on this screen to edit parent client area mapping approach, check the previous  else if (wParam == VK_BACK) block 
                RECT mappedParentRect; 
                GetWindowRect(hwnd, &mappedParentRect);
                MapWindowPoints(HWND_DESKTOP, GetParent(hwnd), (LPPOINT)&mappedParentRect, 2);
                InflateRect(&mappedParentRect, 10, 10);

                // Single redraw operation 
                InvalidateRect(GetParent(hwnd), &mappedParentRect, FALSE);

                // Re-enable and update
                SendMessage(hwnd, WM_SETREDRAW, TRUE, 0);
                RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
                return 0;
            }
            break;
        }

        

        case WM_MOUSEMOVE:
        {
            // Paint selected text when using left mouse an drag to select text portions ot the whole text, in order to prevent text partes or lines to disappear. 
            // TODO: Check what this is doing exactly
            if (GetKeyState(VK_LBUTTON) < 0) 
            {
                LRESULT result = CallWindowProc(oldEditBoxProc, hwnd, msg, wParam, lParam);
                RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
                return result;
            }

            if (!CustomEditBoxState.isHovered)
            {
                CustomEditBoxState.isHovered = true;
                
                // Set up tracking for mouse leave
                TRACKMOUSEEVENT tme = { sizeof(tme) };
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hwnd;
                TrackMouseEvent(&tme);
                
                // Force repaint to show hover state
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return CallWindowProc(oldEditBoxProc, hwnd, msg, wParam, lParam);
        }

        case WM_MOUSELEAVE:
        {
            CustomEditBoxState.isHovered = false;
            
            // Get edit control rect and map it to parent coordinates in one step
            RECT rect;
            GetWindowRect(hwnd, &rect);
            MapWindowPoints(HWND_DESKTOP, GetParent(hwnd), (LPPOINT)&rect, 2);
            
            // Inflate the rect to cover the border area
            InflateRect(&rect, 10, 10);
            
            // Clean parent area and force repaint
            InvalidateRect(GetParent(hwnd), &rect, TRUE);
            
            // Force a complete repaint of the edit control
            InvalidateRect(hwnd, NULL, TRUE);

            return CallWindowProc(oldEditBoxProc, hwnd, msg, wParam, lParam);
        }

        case WM_LBUTTONDOWN:
        {
            CustomEditBoxState.isPressed = true;
            InvalidateRect(hwnd, NULL, TRUE);
            return CallWindowProc(oldEditBoxProc, hwnd, msg, wParam, lParam);
        }

        case WM_LBUTTONUP:
        {
            // Paint selected text when using shift + arrows to select text portions ot the whole text, in order to prevent text partes or lines to disappear. 
            // TODO: Check what this is doing exactly and if we need this here together with the WM_KEYDOWN handling  
            if (GetKeyState(VK_SHIFT) < 0) 
            {
                LRESULT result = CallWindowProc(oldEditBoxProc, hwnd, msg, wParam, lParam);
                RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ALLCHILDREN);
                return result;
            }
            CustomEditBoxState.isPressed = false;
            InvalidateRect(hwnd, NULL, TRUE);
            return CallWindowProc(oldEditBoxProc, hwnd, msg, wParam, lParam);
        }
        
        case WM_PAINT:
        {   
            // In order to paint all the layer elments properly (rect, border, text) we need to get individual DC for each element. This is why we get indvidual DCs here and in the border instead of calling BeginPaint/EndPaint
            HDC hdc = GetDC(hwnd);
            RECT rect;
            GetClientRect(hwnd, &rect);
            
            // Create clipping region for rounded corners
            HRGN hRgn = CreateRoundRectRgn(rect.left, rect.top, 
                                           rect.right, rect.bottom, 
                                           10, 10);

            // SetWindowRgn actually defines the shape of the entire window (in this case, your edit control). Any area outside this region is not just visually transparent - it literally doesn't exist as far as Windows is concerned. The window is literally shaped like a rounded rectangle.
            SetWindowRgn(hwnd, hRgn, TRUE);

            // Background first
            HBRUSH hBrush = CreateSolidBrush(RGB(40,40,40));

            // Draw the rounded background
            HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
            HPEN oldPen = (HPEN)SelectObject(hdc, nullPen);
            HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, hBrush);

            RoundRect(hdc, rect.left, rect.top, rect.right, rect.bottom, 10, 10);
            
            SelectObject(hdc, oldPen);
            SelectObject(hdc, oldBrush);
            DeleteObject(hBrush);
            ReleaseDC(hwnd, hdc);

            // Now let the edit control paint its text ON TOP of our background
            //
            // The original edit control's painting procedure will:
            // Use the colors/settings we specified in WM_CTLCOLOREDIT (text color and transparent background)
            // Paint the actual text content in the control
            // Return a value (typically 0 for WM_PAINT if successful)
            LRESULT result = CallWindowProc(oldEditBoxProc, hwnd, msg, wParam, lParam);

            // Finally paint the border if needed
            if(CustomEditBoxState.isHovered) 
            {
                hdc = GetDC(hwnd);
                HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
                HPEN bluePen = CreatePen(PS_SOLID, 2, RGB(0,122,204));
                oldPen = (HPEN)SelectObject(hdc, bluePen);
                oldBrush = (HBRUSH)SelectObject(hdc, nullBrush);

                // The key is offsetting the hover bord er by 1 pixel on all sides to account for the pen width, while keeping the background rect at the full dimensions. This should maintain consistent border thickness and corner rounding.
                // The adjustment is needed because of how Windows GDI draws lines with different pen widths. 
                RoundRect(hdc, rect.left + 1, rect.top + 1, 
                        rect.right - 1, rect.bottom - 1, 
                        10, 10);

                SelectObject(hdc, oldPen);
                SelectObject(hdc, oldBrush);
                DeleteObject(bluePen);
                ReleaseDC(hwnd, hdc);
            }
            
            return result;
        }

        case WM_SETFOCUS:
        {
            CallWindowProc(oldEditBoxProc, hwnd, msg, wParam, lParam);

            // Create a new caret with custom dimensions
            // NOTE: For window-level caret management, Windows maintains only ONE caret active at a time in the system. When you create a new caret, the old one is automatically destroyed. 
            // To follow the good practice this caret will be properly destroyed in KILLFOCUS
            CreateCaret(hwnd, NULL, 10, 15); // width, height
            ShowCaret(hwnd);
            
            // If edit box is in focus check if any task is in edit mode (isEditing state with the hTempEdit Edit Control) and deactivate the edit state as well as the Edit Control itself to make the task go back to its default state, with white text.   
            for(int i{}; i < CustomListBoxState.tasks.size(); ++i)
            {
                if(CustomListBoxState.tasks[i].isEditing)
                {
                    TaskRect &task = CustomListBoxState.tasks[i];

                    TaskFunctions::resetTaskAnimation(hwnd, &task, FALSE);
                    
                    // Set hovered to true to keep the blue border 
                    task.isHovered = true;

                    // We need to invalidate the listbox in order to repaint the updated positions in case the edited task rects height changed
                    InvalidateRect(hListBox, NULL, TRUE);

                    task.isHovered = false;
                }    
            }
            break;
        }


        case WM_KILLFOCUS:
        {
            HideCaret(hwnd);
            DestroyCaret();
            return CallWindowProc(oldEditBoxProc, hwnd, msg, wParam, lParam);
        }
    }
    
    return CallWindowProc(oldEditBoxProc, hwnd, msg, wParam, lParam);
}

///List Box Subclass //////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////////////////////////



LRESULT CALLBACK ListSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch(msg)
    {
        case WM_ERASEBKGND:
        {
            return TRUE;    // Prevent flickering
        }


        case WM_KEYDOWN:
        {
            // TODO: Check the difference between this and the previous code 
            int selectedIndex = CustomListBoxState.selectedIndex;
            if (selectedIndex >= 0 && selectedIndex < CustomListBoxState.tasks.size()) 
            {
                if (wParam == VK_DOWN || wParam == VK_UP) 
                {


                    // TODO: Tweak the x and y position to match the client area coordinates. 
                    SetCursorPos(maxTaskWidth, 299);

                    int currentIndex = CustomListBoxState.selectedIndex;
                    int newIndex = currentIndex;

                    // Determine the new index based on key press
                    if (wParam == VK_DOWN) 
                    {
                        newIndex = std::min(currentIndex + 1, (int)CustomListBoxState.tasks.size() - 1);
                    } 
                    else if (wParam == VK_UP) 
                    {
                        newIndex = std::max(currentIndex - 1, 0);
                    }

                    if (newIndex != currentIndex) 
                    {   
                        // ANTI FLICKERING APPROACH // 
                        /////////////////////////////
                        // IMPORTANT: To stop the flickering on previously selected tasks, we need to make this call to set the LISTBOX internal selection state to the newIndex (or -1 for no selection). We need to do this because the list box maintains its internal selection state, which overrides our custom reset logic.
                        // To resolve this, we need to explicitly clear the list box's internal selection state whenever a new task is selected.
                        // 
                        // Reset the internal selection of the list box
                        SendMessage(hwnd, LB_SETCURSEL, (WPARAM)newIndex, 0);

    
                        // Together with the internal selection, we aslo need to make sure that we invalidate the exact bounds only of the old and new task rects (and immediatley update the wndow to force repaint). this also reduces the flickering quite considerably  

                        // Reset the previous task's state
                        if (currentIndex >= 0 && currentIndex < CustomListBoxState.tasks.size()) 
                        {
                            TaskRect &prevTask = CustomListBoxState.tasks[currentIndex];
                            TaskFunctions::resetTaskAnimation(hwnd, &prevTask, false);
                            prevTask.isHovered = false;
                            prevTask.isPressed = false;

                            // Invalidate the previous task's rectangle
                            RECT oldTaskRect;
                            SendMessage(hwnd, LB_GETITEMRECT, currentIndex, (LPARAM)&oldTaskRect);
                            oldTaskRect.right = maxTaskWidth;
                            InvalidateRect(hwnd, &oldTaskRect, FALSE);                            
                            // UpdateWindow(hwnd);

                        }

                        // Update the new task's state
                        TaskRect &newTask = CustomListBoxState.tasks[newIndex];
                        newTask.isHovered = true;
                        newTask.isPressed = true;

                        // Calculate data in order to simulate a click on the selected task and triggerthe blue char animation properly 
                        ////////////////////////////////////////
                        ///////////////////////////////////////

                        // // Calculate x using task bounds as before
                        // // IMPORTANT: Fist we need to get the actual task bound values because the ones coming from the LB_GETITEMRECT message return the task values with full width of the listbox. 
                        // int xPos = newTask.bounds.left + ((newTask.bounds.right - newTask.bounds.left) / 2);
                        
                        // // Use task bounds for Y calculation too
                        // // IMPORTANT: We also need to get the actual task bound for the Y values since the LB_GETITEMRECT also returns the Y values with internal modifications which do not give us the actual area where the custom task bounds are....
                        // int yPos = newTask.bounds.top + ((newTask.bounds.bottom - newTask.bounds.top) / 2);

                        // LPARAM mouseParam = MAKELPARAM(xPos, yPos);

                        LPARAM mouseParam = HelperFunctions::clickTaskXYPos(&newTask.bounds);

                        // Simulate pressing the task 
                        SendMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, mouseParam);

                        //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


                        // Invalidate the new task's rectangle
                        RECT newTaskRect;
                        SendMessage(hwnd, LB_GETITEMRECT, newIndex, (LPARAM)&newTaskRect);
                        newTaskRect.right = maxTaskWidth;
                        InvalidateRect(hwnd, &newTaskRect, FALSE);
                        // UpdateWindow(hwnd); 

                        // IMPORTANT: We need this to make sure the proper selected tasks are visible during scrolling 
                        // Ensure the new task is visible
                        SendMessage(hwnd, LB_SETTOPINDEX, newIndex, 0);

                        // Clean current frame from background before painting new frames //
                        ////////////////////////////////////////////////////////////////////

                        // HDC DeviceContext = GetDC(hwnd);

                        // // Paint black background
                        // RECT clientRect;
                        // GetClientRect(hwnd, &clientRect);
                        // HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
                        // clientRect.left = 300;
                        // clientRect.right = 600;
                        // clientRect.bottom = 50; 
                        // FillRect(DeviceContext, &clientRect, blackBrush);
                        // DeleteObject(blackBrush);


                        ////////////////////////////
                        // Single animation block //
                        ////////////////////////////

                         // // Load animation frame
                        // // GlobalAnimation.Frames = FrameLoader::LoadFrameSequence(L"frames2");
                        // GlobalAnimation.CurrentFrame = 0;
                        // // GlobalAnimation.IsPlaying = true;
                        // // GlobalAnimation.TimerID = ANIMATION_START_TIMER;

                        // win32_window_dimension Dimension = Win32GetWindowDimension(hwnd);
                        // UpdateWindowSingleFrame(DeviceContext, Dimension.Width, Dimension.Height);
                        

                        // //////////////////////////////
                        // // Multiple animation block //
                        // //////////////////////////////

                        //  // Load animation frames
                        // GlobalAnimation.cleanup();
                        // GlobalAnimation.Frames = FrameLoader::LoadFrameSequence(L"framesBlue");
                        // GlobalAnimation.CurrentFrame = 0;
                        // GlobalAnimation.IsPlaying = true;
                        // GlobalAnimation.TimerID = ANIMATION_START_TIMER;

                        // // If frames loaded successfully, start the timer
                        // if(!GlobalAnimation.Frames.empty() && GlobalAnimation.IsPlaying)
                        // {
                        //     SetTimer(GetParent(hwnd), GlobalAnimation.TimerID, 
                        //             60, NULL);
                        //     SetTimer(GetParent(hwnd), ANIMATION_STOP_TIMER, 600, NULL);
                        // }





                        // Update the selected index
                        CustomListBoxState.selectedIndex = newIndex;


                    }

                    return 0; // Mark the message as handled
                }
                // // Block that does not scroll the task list 
                // if (wParam == VK_DOWN || wParam == VK_UP) 
                // {
                //     int currentIndex = CustomListBoxState.selectedIndex;
                //     int newIndex = (wParam == VK_DOWN) 
                //                     ? min(currentIndex + 1, (int)CustomListBoxState.tasks.size() - 1)
                //                     : max(currentIndex - 1, 0);

                //     if (newIndex != currentIndex) 
                //     {   
                //         // **1. Reset Previous Task's State Without Repainting the Full Row**
                //         TaskRect &prevTask = CustomListBoxState.tasks[currentIndex];
                //         TaskFunctions::resetTaskAnimation(hwnd, &prevTask, false);
                //         prevTask.isHovered = false;
                //         prevTask.isPressed = false;

                //         RECT oldTaskRect;
                //         SendMessage(hwnd, LB_GETITEMRECT, currentIndex, (LPARAM)&oldTaskRect);
                //         oldTaskRect.right = min(oldTaskRect.right, 300);
                //         InvalidateRect(hwnd, &oldTaskRect, FALSE);

                //         // **2. Update the New Task's State**
                //         TaskRect &newTask = CustomListBoxState.tasks[newIndex];
                //         newTask.isHovered = true;
                //         newTask.isPressed = true;

                //         // **3. Ensure the New Task is Fully Visible**
                //         int topIndex = SendMessage(hwnd, LB_GETTOPINDEX, 0, 0);
                //         int visibleCount = SendMessage(hwnd, LB_GETCOUNT, 0, 0);
                        
                //         // Scroll up if the new index is above the top visible item
                //         if (newIndex < topIndex) 
                //         {
                //             SendMessage(hwnd, LB_SETTOPINDEX, newIndex, 0);
                //         }
                //         // Scroll down if the new index is below the last visible item
                //         else 
                //         {
                //             RECT newTaskRect;
                //             SendMessage(hwnd, LB_GETITEMRECT, newIndex, (LPARAM)&newTaskRect);

                //             // Get the bottom of the last visible task
                //             RECT lastVisibleTaskRect;
                //             SendMessage(hwnd, LB_GETITEMRECT, topIndex + visibleCount - 1, (LPARAM)&lastVisibleTaskRect);

                //             if (newTaskRect.bottom > lastVisibleTaskRect.bottom) 
                //             {
                //                 SendMessage(hwnd, LB_SETTOPINDEX, newIndex - (visibleCount - 1), 0);
                //             }
                //         }

                //         // **4. Invalidate Only the New Task Rect (Preventing Background Erasure)**
                //         RECT newTaskRect;
                //         SendMessage(hwnd, LB_GETITEMRECT, newIndex, (LPARAM)&newTaskRect);
                //         newTaskRect.right = min(newTaskRect.right, 300);
                //         InvalidateRect(hwnd, &newTaskRect, FALSE);

                //         // **5. Update the Selection Index Without Triggering a Full Repaint**
                //         CustomListBoxState.selectedIndex = newIndex;
                //     }

                //     return 0; // Mark message as handled
                // }
                // Edit task 
                else if (wParam == VK_F2)
                {
                    // Hide mouse cursor 
                    ShowCursor(FALSE);
                    SetCapture(hwnd);


                    TaskRect &task = CustomListBoxState.tasks[CustomListBoxState.selectedIndex];
                            
                    LPARAM mouseParam = HelperFunctions::clickTaskXYPos(&task.bounds);

                    // Simulate pressing the task first and then, double click at the calculated position
                    // IMPORTANT: It is crucial to select the task first (simulating a button click on it) in order to properly enter editing mode and properly paint all the correct bounds for the tasks. 
                    SendMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, mouseParam);
                    // Simulate and trigger double click to enter editing mode 
                    SendMessage(hwnd, WM_LBUTTONDBLCLK, MK_LBUTTON, mouseParam);
                    
                    InvalidateRect(hwnd, NULL, FALSE);
                    UpdateWindow(hwnd);

                    // // Make mouse cursor visible again
                    // ShowCursor(TRUE);
                    // ReleaseCapture();

                    return 0;
                } 
                // Set new deadline 
                else if (wParam == VK_F1)
                {              
                    // Call the Deadline Dialog control 
                    ShowDarkDialog(hwnd);

                    // TODO: Also, lets make some bool that marks the type of dreadline that was chosen in the TaskRect members and then paints some animation on the task rect with the respective color to show what type of deadline it is
                    TaskRect &task = CustomListBoxState.tasks[selectedIndex];
                          
                    LPARAM mouseParam = HelperFunctions::clickTaskXYPos(&task.bounds);

                    // Simulate pressing the task 
                    SendMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, mouseParam);
                    
                    InvalidateRect(hwnd, NULL, FALSE);
                    UpdateWindow(hwnd);

                    return 0;
                }
                // Check deadline
                else if(wParam == VK_F3)
                {
                    ShowCountDialog(hwnd);

                    // TODO: Also, lets make some bool that marks the type of dreadline that was chosen in the TaskRect members and then paints some animation on the task rect with the respective color to show what type of deadline it is
                    TaskRect &task = CustomListBoxState.tasks[selectedIndex];

                    LPARAM mouseParam = HelperFunctions::clickTaskXYPos(&task.bounds);
                    
                    // Simulate pressing the task first and then, double click at the calculated position
                    // IMPORTANT: It is crucial to select the task first (simulating a button click on it) in order to properly enter editing mode and properly paint all the correct bounds for the tasks. 
                    SendMessage(hwnd, WM_LBUTTONDOWN, MK_LBUTTON, mouseParam);
                    
                    InvalidateRect(hwnd, NULL, FALSE);
                    UpdateWindow(hwnd);

                    return 0;
                }
            }
            break; 
        }

        case WM_MOUSEMOVE:
        {
            // After pressing up / down / f2 to handle tasks with arrow keys, show cursor again once we move the mouse 
            ShowCursor(TRUE);
            ReleaseCapture();

            static POINT lastPt = {0, 0};
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

            // ANTI-Flickering approach!
            ////////////////////////////
            // Only process if mouse moved more than 2 pixels in any direction
            // IMPORTANT: This is crucial to avoid the constant random flickering on random tasks due to constant (unnecesary) process and state changes
            if(abs(pt.x - lastPt.x) < 2 && abs(pt.y - lastPt.y) < 2 /* || isScrolling */)
            {
                break;
            }
            lastPt = pt;
            
            int hoveredIndex = TaskFunctions::findTaskUnderMouse(hwnd, pt);

            // If we found a task under the mouse
            if(hoveredIndex >= 0)
            {   
                // Only proceed if this isn't already the hovered task
                if(hoveredIndex != CustomListBoxState.hoveredIndex)
                {   
                    // ANTI-Flickering approach
                    //////////////////////////
                    // Define what are the regions of the rects that need update painting and only invalidate these specific ones!  
                    RECT updateRect = {0};
                    
                    // Get old hover rect if exists
                    // Clear previous hover state if any
                    if(CustomListBoxState.hoveredIndex >= 0 && 
                    CustomListBoxState.hoveredIndex < CustomListBoxState.tasks.size())
                    {
                        CustomListBoxState.tasks[CustomListBoxState.hoveredIndex].isHovered = false;
                        SendMessage(hwnd, LB_GETITEMRECT, 
                                    CustomListBoxState.hoveredIndex, (LPARAM)&updateRect);                     
                    }

                    // Get new hover rect
                    RECT newRect;
                    SendMessage(hwnd, LB_GETITEMRECT, hoveredIndex, (LPARAM)&newRect);
                    
                    // ANTI-Flickering approach
                    // Combine rects if we had a previous hover
                    if(updateRect.bottom != 0)
                    {
                        UnionRect(&updateRect, &updateRect, &newRect);
                    }
                    else
                    {
                        updateRect = newRect;
                    }
                    
                    // Set new hover state
                    CustomListBoxState.tasks[hoveredIndex].isHovered = true;
                    CustomListBoxState.hoveredIndex = hoveredIndex;
                    
                    // Set up mouse tracking
                    TRACKMOUSEEVENT tme = { sizeof(tme) };
                    tme.dwFlags = TME_LEAVE;
                    tme.hwndTrack = hwnd;
                    TrackMouseEvent(&tme);
                                    
                    // ANTI-Flickering approach
                    // Only invalidate the affected areas
                    InvalidateRect(hwnd, &updateRect, FALSE);
                    // InvalidateRect(hwnd, NULL, FALSE);
                }
            }
            // Else if no task was found under the mouse position (if hoveredIndex is -1). E.g. if we click on the edit box while one of the tasks is still hovered, we need to clear it. 
            // 
            // This approach leverages the fact that we're tracking which task is currently hovered. If hoveredIndex is >= 0, it means we know exactly which task was previously hovered and can directly access it without searching.
            else if(CustomListBoxState.hoveredIndex >= 0)
            {
                RECT updateRect;
                SendMessage(hwnd, LB_GETITEMRECT, CustomListBoxState.hoveredIndex, 
                        (LPARAM)&updateRect);
                
                CustomListBoxState.tasks[CustomListBoxState.hoveredIndex].isHovered = false;
                CustomListBoxState.hoveredIndex = -1;
                // CustomListBoxState.selectedIndex = -1;
                
                // ANTI-Flickering approach
                InvalidateRect(hwnd, &updateRect, FALSE);
            }
            break;
        }

        case WM_MOUSELEAVE:
        {
            for(TaskRect &task : CustomListBoxState.tasks)
            {
                if(task.isHovered)
                {
                    task.isHovered = false;
                    CustomListBoxState.hoveredIndex = -1;       

                    // InvalidateRect(hwnd, NULL, TRUE);


                    if (CustomListBoxState.hoveredIndex >= 0)
                    {
                        RECT hoverRect;
                        SendMessage(hwnd, LB_GETITEMRECT, CustomListBoxState.hoveredIndex, (LPARAM)&hoverRect);
                        hoverRect.right = std::min((int)hoverRect.right, 300);
                        InvalidateRect(hwnd, &hoverRect, FALSE);
                    }
                }
            }
            break;
        }

        // case WM_VSCROLL:
        case WM_MOUSEWHEEL:
        {
            // Check if any task is in editing mode or if it is selected (with fully blue characters)
            //
            // We ended up adding also the selected tasks to avoid the lack of redrawing that is set in WM_DRAWITEM in main which prevents flickering and repainting for the text that is fully blue!
            for(TaskRect &task : CustomListBoxState.tasks)
            {
                
                //  // Start frame animation implementation  
                // if(!GlobalAnimation.Frames.empty()) 
                // {
                //     GlobalAnimation.IsPlaying = true;
                //     GlobalAnimation.CurrentFrame = 0;  
                //     SetTimer(GetParent(hwnd), GlobalAnimation.TimerID, ANIMATION_FRAME_DURATION, NULL);
                //     SetTimer(GetParent(hwnd), ANIMATION_STOP_TIMER, 1000, NULL);
                // }


                // If task is in editing mode (pink chars), block the scrolling possibility!
                if(task.isEditing)
                {
                    return 0;
                }
                // else, if task is selected (fully blue chars) reset its active painted states and invalidate it to set it back to the default appearance (white text) before actually starting the VSCROLL procedure with CallWindowProc(oldListBoxProc, hwnd, msg, wParam, lParam);
                else if(task.fullyBlue)
                {   
                    ///////////////////////////////////////////////////////////////////////
                    // TODO: Leaving this here for documentation and examplification of how to use similar debugging techniques.  
                    /////////////////////////////////////////////////////////////////////////
                     DebugTaskState(task, "before-scroll-reset");
                    // Is the timer actually getting killed?
                    if(task.timerId)
                    {
                        // TODO: Leaving this here for documentation and examplification of how to use similar debugging techniques.  
                        char debug[256];
                        sprintf_s(debug, "Active timer found: %zu\n", task.timerId);
                        OutputDebugStringA(debug);
                    }

                    // Force focus to main window which will trigger WM_KILLFOCUS and reset selected list box task item, preventing its constant flickering due to constant repaints for that selection  
                    SetFocus(GetParent(hwnd));
                    
                    // Now let the scroll happen
                    return CallWindowProc(oldListBoxProc, hwnd, msg, wParam, lParam);   

                    // TODO: Leaving this here for documentation and examplification of how to use similar debugging techniques.  
                    DebugTaskState(task, "after-scroll-reset");
                }     
            }
        
            // Important to invalidate without clearing the backgound, so we can keep whatever is painted on the main window (my frames for example)
            InvalidateRect(hwnd, NULL, FALSE); // Forces repaint without erasing background
            return CallWindowProc(oldListBoxProc, hwnd, msg, wParam, lParam);
            // break;
        }
  
        case WM_LBUTTONDOWN:
        {   
            // Get exact mouse click coordinates relative to the listbox
            // We place this here because we might want to use these values for other purposes than the CustomListBoxState tasks rect index position 
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam)};
            int hitIndex = TaskFunctions::findTaskUnderMouse(hwnd, pt);

            // While task is in edit mode, deactivate the single click trigger by not entering this condition 
            //
            // adding fullyBlue bool check to avoid triggering paint animation procedure when task is already selected and fully painted in blue 
            if(hitIndex >= 0 && hitIndex < CustomListBoxState.tasks.size() && 
            !CustomListBoxState.tasks[hitIndex].isEditing && !CustomListBoxState.tasks[hitIndex].fullyBlue) 
            {   
                // ANTI-Flickering approach!
                // Also helps properly repainting selected items when list scrolls automatcally when chosing a partially painted task that does not fit the displayed list box are
                ////////////////////////////
                // Calculate exact regions we need to update
                RECT updateRect = {0};
                
                // Get rectangle for previously selected task if exists
                if(CustomListBoxState.selectedIndex >= 0 && 
                CustomListBoxState.selectedIndex < CustomListBoxState.tasks.size())
                {
                    SendMessage(hwnd, LB_GETITEMRECT, CustomListBoxState.selectedIndex, 
                                (LPARAM)&updateRect);
                }

                // Get rectangle for newly clicked task
                RECT clickedRect;
                SendMessage(hwnd, LB_GETITEMRECT, hitIndex, (LPARAM)&clickedRect);
                
                // ANTI-Flickering approach!
                ////////////////////////////
                // Combine the regions if needed
                if(updateRect.bottom != 0)
                {
                    // The UnionRect call combines these two areas into a single rectangle that covers both the old and new selection. 
                    // For example, if you have:

                    // Previous selection at coordinates (10,10,100,30)
                    // New selection at coordinates (50,20,150,40)

                    // UnionRect would create a single rectangle at (10,10,150,40) that encompasses both areas. This is more efficient than redrawing two separate rectangles.
                    UnionRect(&updateRect, &updateRect, &clickedRect);
                }
                else
                {
                    updateRect = clickedRect;
                }

                // Make state changes
                CustomListBoxState.clickedIndex = hitIndex;
                CustomListBoxState.tasks[hitIndex].isPressed = true;


                // TODO: Implement different animations for task button click
                // TODO: Probably we are going to need to load a different set of animations with the LoadFrameSequence() function
                //  
                // // Start frame animation implementation  
                // if(!GlobalAnimation.Frames.empty()) 
                // {
                //     GlobalAnimation.IsPlaying = true;
                //     GlobalAnimation.CurrentFrame = 0;  
                //     SetTimer(GetParent(hwnd), GlobalAnimation.TimerID, ANIMATION_FRAME_DURATION, NULL);
                //     SetTimer(GetParent(hwnd), ANIMATION_STOP_TIMER, 800, NULL);
                // }
                
                // Reset previous task's animation
                if(CustomListBoxState.selectedIndex >= 0 && 
                CustomListBoxState.selectedIndex < CustomListBoxState.tasks.size())
                {
                    TaskRect &prevTask = CustomListBoxState.tasks[CustomListBoxState.selectedIndex];
                    TaskFunctions::resetTaskAnimation(hwnd, &prevTask, false);
                }

                // Start blue character animation for newly selected task
                TaskRect &task = CustomListBoxState.tasks[CustomListBoxState.clickedIndex];
                
                // Call this Task Rect function to get all the word boundary positions so we can properly paint and increment blue chars with the timer/drawitem logic. 
                task.calculateWordBoundaries();
                task.blueCharCount = 0;
                // Set timer ID to be 1000 + the clicked index so we can then sub 1000 in the TIMER case and immediately get the index value (pretty neat trick) 
                task.timerId = SetTimer(hwnd, 1000 + CustomListBoxState.clickedIndex, 25, NULL);
                
             
                CustomListBoxState.selectedIndex = hitIndex;


                // Check if task is expired or not to select the cirrect frame set
                if(!task.expired)
                {
                        // IMPORTANT: clean the frame vector before loading new elements to avoid memory leaks!
                    // GlobalAnimation.cleanup();
                    // Load Blue animation frames
                    // GlobalAnimation.Frames = FrameLoader::LoadFrameSequence(L"framesBlue");

                    GlobalAnimation.CurrentFrame = 0;
                    GlobalAnimation.IsPlaying = true;
                    GlobalAnimation.frameVector = L"FramesBlue";
                    GlobalAnimation.TimerID = ANIMATION_START_TIMER;

                    // If frames loaded successfully, start the timer
                    if(!GlobalAnimation.FramesRed.empty() && GlobalAnimation.IsPlaying)
                    {
                        SetTimer(GetParent(hwnd), GlobalAnimation.TimerID, 
                                60, NULL);
                        SetTimer(GetParent(hwnd), ANIMATION_STOP_TIMER, 600, NULL);
                    }
                    
                }
                else
                {
                    // IMPORTANT: clean the frame vector before loading new elements to avoid memory leaks!
                    // GlobalAnimation.cleanup();
                    // Load Red animation frames
                    // GlobalAnimation.Frames = FrameLoader::LoadFrameSequence(L"framesRed");


                    GlobalAnimation.CurrentFrame = 0;
                    GlobalAnimation.IsPlaying = true;
                    GlobalAnimation.frameVector = L"FramesRed";
                    GlobalAnimation.TimerID = ANIMATION_START_TIMER;
 
                    if(!GlobalAnimation.FramesBlue.empty() && GlobalAnimation.IsPlaying)
                    {
                        SetTimer(GetParent(hwnd), GlobalAnimation.TimerID, 
                                70, NULL);
                        SetTimer(GetParent(hwnd), ANIMATION_STOP_TIMER, 4215, NULL);
                    }

                }


                // ANTI-Flickering approach!
                ////////////////////////////
                // Also helps properly repainting selected items when list scrolls automatcally when chosing a partially painted task that does not fit the displayed list box are
                //////////////////////////////////////////
                // Only invalidate our calculated region
                InvalidateRect(hwnd, &updateRect, FALSE);
            } 
             
            SetFocus(hwnd);
            SetCapture(hwnd);
            break;
        }


        case WM_LBUTTONUP:
        {
            if(!CustomListBoxState.tasks.empty() &&
               CustomListBoxState.clickedIndex >= 0 && 
               CustomListBoxState.clickedIndex < CustomListBoxState.tasks.size())
            {
                CustomListBoxState.tasks[CustomListBoxState.clickedIndex].isPressed = false;
            }
            ReleaseCapture();
            // Unnecessary invalidation call here, LBUTTONDOWN and TIMER are already forcing repaints 
            // InvalidateRect(hwnd, NULL, FALSE);
            break;
        }

        case WM_LBUTTONDBLCLK:
        {
            POINT pt = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };

            //  IMPORTANT: The key to get the actual task item with the scroll vscroll implementation is using Windows' built-in list box item position tracking (LB_ITEMFROMPOINT used in the findTaskUnderMouise() call ad LB_GETITEMRECT usedin the SendMessage() below) instead of trying to track positions manually.
            // The key takeaway is that the code relies on the list box control's built-in item tracking functionality (LB_ITEMFROMPOINT and LB_GETITEMRECT) to determine the clicked task item based on the scroll position. This approach is more reliable than manually tracking item positions, as the list box control handles the scroll position and item coordinates internally.
            int hitIndex = TaskFunctions::findTaskUnderMouse(hwnd, pt);
            
            if(hitIndex >= 0 && hitIndex < CustomListBoxState.tasks.size() && CustomListBoxState.tasks[hitIndex].fullyBlue)
            {
                TaskRect &task = CustomListBoxState.tasks[hitIndex];
                task.isEditing = true;
                CustomListBoxState.selectedIndex = hitIndex;

                // Get task position in client coordinates
                RECT itemRect;
                //  IMPORTANT: The key to get the actual task item with the scroll vscroll implementation is using Windows' built-in list box item position tracking (LB_GETITEMRECT) instead of trying to track positions manually.
                //  For this specific double click mouse button to enter edit mode with Edit text Control, we need to make this SendMessage call in order to get the ItemRect bounds depending on the scroll position
                SendMessage(hwnd, LB_GETITEMRECT, hitIndex, (LPARAM)&itemRect);
                // IMPORTANT: Because we need to use this call to get the actual task position and bounds in relation to the scroll events. By getting the positions and bounds from GETITEMRECT directly, we have to account for the following behaviour:

                // - Windows is now managing the item positions internally
                // - LB_GETITEMRECT returns the full width of the list box control for each item
                // -  The scroll position affects where items are displayed
                // The itemRect from LB_GETITEMRECT spans the entire width of the list box, including the scroll bar area

                // Hence, we need to calculate the rects in a different way, not relying on the calculations made in the DRAWITEM from main any longer and adjusting the positions and dimensions we are getting from GETITEMRECT.  

                // Calculate and adjust proper bounds we got from GETITEMRECT based on task width logic from DRAWITEM
                itemRect.right = (task.bounds.right > maxTaskWidth) ? maxTaskWidth : task.bounds.right;

                // Map to parent coordinates for edit control
                MapWindowPoints(hwnd, GetParent(hwnd), (LPPOINT)&itemRect, 2);

                int taskHeight = itemRect.bottom - itemRect.top;
                
                task.hTempEdit = CreateWindowEx(0,
                                                L"EDIT",
                                                task.text.c_str(),
                                                WS_CHILD | WS_VISIBLE | ES_AUTOVSCROLL | 
                                                ES_MULTILINE | ES_LEFT| WS_CLIPCHILDREN | WS_CLIPSIBLINGS,
                                                // Before VScroll implemention
                                                // itemRect.left + taskXPadding - 4,
                                                itemRect.left + (2 * taskXPadding) - 4,
                                                itemRect.top + taskYSpacing,
                                                // Before VScroll implemention
                                                // (itemRect.right - itemRect.left) - taskXPadding,
                                                (itemRect.right - itemRect.left) - (2 * taskXPadding),
                                                // Before VScroll implemention
                                                // taskHeight - (2 * taskYSpacing ) - 4,
                                                taskHeight - (3 * taskYSpacing) - 4,
                                                GetParent(hwnd),
                                                (HMENU)ID_EDITBOX,
                                                GetModuleHandle(NULL),
                                                NULL);
                
                    if(task.hTempEdit == NULL) 
                    {
                        // Get the error code and format an error message
                        DWORD error = GetLastError();
                        wchar_t msg[256];
                        swprintf_s(msg, L"Failed to create edit control. Error code: %d", error);
                        MessageBox(NULL, msg, L"Error", MB_OK | MB_ICONERROR);
                        return 0;
                    }

                    // Set font, focus and force the Edit Control to immeditely enter edit mode and show the caret
                    if(task.hTempEdit)
                    {

                        // Set pointer to the task data so we can pass it to the subclass call as the dwRefData
                        TaskRect *ptask = &task;

                        // Set the subclass after creating the control
                        // IMPORTANT: Note that we pass a pointer to the task data to the subclass as dwRefData arg, so we can simply access the task data used for the Edit Control and manipulate them as we wish  
                        SetWindowSubclass(task.hTempEdit, TempEditSubclassProc, 1, (DWORD_PTR)ptask);

                        SendMessage(task.hTempEdit, WM_SETFONT, (WPARAM)hGlobalFont, TRUE);
                        
                        // First activate and focus
                        SetActiveWindow(task.hTempEdit);
                        SetFocus(task.hTempEdit);
                        
                        // Get edit control's window rect
                        RECT editRect;
                        GetWindowRect(task.hTempEdit, &editRect);
                        
                        // Calculate center point of edit control
                        // define the exact center so we can be sure the task region we want will be clicked 
                        POINT clickPt;
                        clickPt.x = (editRect.right - editRect.left) / 2;
                        clickPt.y = (editRect.bottom - editRect.top) / 2;
                        
                        // Convert to client coordinates
                        // Remember that we set the Special Edit Control to have the main window as the parent (check the CreateWindowExW() above). Because of this we need to convert the coordinates to be relative to the actual Edit Control  
                        POINT clientPt = clickPt;
                        //  ScreenToClient is used here because it uses the Screen area as the default source. We could have used MapWindowPoints and set both src and destination client areas, but for this specific need its simpler to just call this ScreenToClient
                        ScreenToClient(task.hTempEdit, &clientPt);
                        
                        // Post (don't send) the mouse messages to ensure proper message order
                        // 
                        // Post is asynchronous - it puts the message in the window's message queue and returns immediately while SendMeasage(is synchronous - it waits for the message to be processed before returning)
                        // For mouse click simulation, PostMessage is often better because it better mimics real mouse input. Real mouse clicks generate messages that get queued in order, they don't process immediately.
                        PostMessage(task.hTempEdit, WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(clientPt.x, clientPt.y));
                        PostMessage(task.hTempEdit, WM_LBUTTONUP, 0, MAKELPARAM(clientPt.x, clientPt.y));
                        PostMessage(task.hTempEdit, WM_SETFOCUS, 0, 0);
                        
                        // Ensure it stays on top!
                        // We need to force the special Edit Control to appear in front so we can see the actual text we are going to edit (and the custom color we defined to it in the CTLCOLOREDIT case in the main winproc ) 
                        SetWindowPos(task.hTempEdit, HWND_TOP, 0, 0, 0, 0, 
                                    SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);

                        // **NOTE**: More explantions about this in the "Forcing Caret and Edit Mode in TempEdit Control" chat of Claude's project 
                    }
                    // break;
                }
                break;
        }
        
        

        case LB_ADDSTRING:
        {
            LRESULT result = CallWindowProc(oldListBoxProc, hwnd, msg, wParam, lParam);

            // Create new task and set its properties
            TaskRect newTask;
            newTask.data.text = reinterpret_cast<LPCWSTR>(lParam);
            // newTask.data.activeDeadline = false;
            newTask.text = reinterpret_cast<LPCWSTR>(lParam);
            newTask.isHovered = false;
            newTask.isPressed = false;
            newTask.hasFocus  = false;

            // Add to local list and save immediately
            try
            {
                // Add to vector
                CustomListBoxState.tasks.push_back(newTask);

                // Debug the save operation
                char debug[256];
                sprintf_s(debug, "Saving %zu tasks to storage\n", CustomListBoxState.tasks.size());
                OutputDebugStringA(debug);

                // Save task
                TaskStorage::saveTasksToFile(CustomListBoxState.tasks);
                OutputDebugStringA("Tasks saved successfully\n");
            }
            catch(const std::exception& e)
            {
                std::string errorMsg = "Failed to save task: ";
                errorMsg += e.what();
                OutputDebugStringA(errorMsg.c_str());
                MessageBoxA(NULL, errorMsg.c_str(), "Error", MB_OK | MB_ICONERROR);
            }

            InvalidateRect(hwnd, NULL, TRUE);

            // returning the size of the task vector as this is the convention expected by the Windws API for LB_ADDSTRING 
            // Returning the index of the newly added item is important because it allows the caller of the LB_ADDSTRING message to perform further operations on the new item if needed, such as setting additional properties or retrieving its data.
            return CustomListBoxState.tasks.size() - 1;   
        }



        case LB_DELETESTRING:
        {
            int index = (int)wParam;
            if(index >= 0 && index < CustomListBoxState.tasks.size())
            {
                // Calculate the entire area that needs to be invalidated
                RECT invalidRect = CustomListBoxState.tasks[index].bounds;

                // Extend invalidRect to include all tasks below the deleted one
                for(size_t i = index + 1; i < CustomListBoxState.tasks.size(); i++) 
                {
                    invalidRect.bottom = CustomListBoxState.tasks[i].bounds.bottom;
                }

                // Convert to parent coordinates before deleting the task
                HWND parentHwnd = GetParent(hwnd);
                POINT points[2] = { {invalidRect.left, invalidRect.top},
                                    {maxTaskWidth, invalidRect.bottom + taskYSpacing} };
                
                MapWindowPoints(hwnd, parentHwnd, points, 2);
                
                RECT parentInvalidRect = { points[0].x, points[0].y,
                                           points[1].x, points[1].y };

                // Delete locally and save updated state
                try
                {   
                    // begin() points to the first element of the vector, and adding index advances the iterator by index positions.
                    CustomListBoxState.tasks.erase(CustomListBoxState.tasks.begin() + index);
                    TaskStorage::saveTasksToFile(CustomListBoxState.tasks);
                }
                catch(const std::exception& e)
                {
                    std::cerr << "Failed to save after deletion: " << e.what() << '\n';
                }

                // Update selection states
                if(CustomListBoxState.tasks.empty())
                {
                    CustomListBoxState.selectedIndex = -1;
                }
                else if(CustomListBoxState.selectedIndex >= CustomListBoxState.tasks.size())
                {
                    CustomListBoxState.selectedIndex--;
                }

                DestroyCaret();

                // Trigger repaints with our properly calculated invalid rect
                InvalidateRect(parentHwnd, &parentInvalidRect, TRUE);
                InvalidateRect(hwnd, NULL, TRUE);
            }
            break;
        }

        case LB_RESETCONTENT:
        {
            // Add to local list and save
            try
            {
                CustomListBoxState.tasks.clear();
                TaskStorage::saveTasksToFile(CustomListBoxState.tasks);
            }
            catch(const std::exception &e)
            {
                std::cerr << "Failed to save cleared tasks state: " << e.what() << '\n';
            }


            // CustomListBoxState.tasks.clear();
            // Reset all the members of the 
            CustomListBoxState.clickedIndex = -1;
            CustomListBoxState.hoveredIndex = -1;
            CustomListBoxState.selectedIndex = -1;
            HideCaret(hwnd);
            InvalidateRect(GetParent(hwnd), NULL, TRUE);
            break;
        }

        // Every time the timer from WM_LBUTTONDOWN ticks, we intercept message sent to TIMER here and increment blueCharCount. This counter tracks how many characters should be blue. The InvalidateRect call forces Windows to repaint the window.
        case WM_TIMER:
        {
            // Get task index from timer ID
            // remember what we did in the WM_LBUTTONDOWN... 1000 + CustomListBoxState.clickedIndex
            int taskIndex = wParam - 1000;

            if(taskIndex >= 0 && taskIndex < CustomListBoxState.tasks.size())
            {
                TaskRect &task = CustomListBoxState.tasks[taskIndex];
                
                // bool to check if the blue chars painting logic needs to be updated or not
                bool needsUpdate = false;


                // Calculate chars to color per tick based on length
                int charsPerTick = 1;
                int textLength = task.text.length();

                if(textLength > 40) {
                    charsPerTick = 15;
                }
                else if(textLength > 25) {
                    charsPerTick = 10;
                }
                else if(textLength > 15) {
                    charsPerTick = 3;
                }

                // Increment with boundary check 
                int newCount = task.blueCharCount + charsPerTick;
                // task.blueCharCount = (newCount <= textLength) ? newCount : textLength;

                // TODO: Check why is the blueCharCount increasing like crazy 
                task.blueCharCount = std::min(newCount, textLength);

                if(task.blueCharCount < textLength)
                {
                    // Find the next word boundary
                    size_t nextSpace = task.text.find_first_of(L" \t\r\n", task.blueCharCount);
                    if(nextSpace == std::wstring::npos) 
                    {
                        task.blueCharCount = textLength;
                    } 
                    else 
                    {
                        // Move to the start of the next word
                        task.blueCharCount = nextSpace + 1;
                    }

                    needsUpdate = true;
                }
                else
                {
                    KillTimer(hwnd, task.timerId);
                    task.timerId = 0;
                    needsUpdate = true;
                }

                if(needsUpdate)
                {
                    RECT itemRect;
                    SendMessage(hwnd, LB_GETITEMRECT, taskIndex, (LPARAM)&itemRect);
                    InvalidateRect(hwnd, &itemRect, FALSE);
                }
            }

            break;
        }


        // TODO: TIMER case with quite a few debug calls, keeping this here as it can be useful to check debegging techniques and investigate the issue with the huge valu we are getting in the blueCharCount variable. 
        //////////////////////////////////////////
        // // Every time the timer from WM_LBUTTONDOWN ticks, we intercept message sent to TIMER here and increment blueCharCount. This counter tracks how many characters should be blue. The InvalidateRect call forces Windows to repaint the window.
        // case WM_TIMER:
        // {
        //     // Get task index from timer ID
        //     // remember what we did in the WM_LBUTTONDOWN... 1000 + CustomListBoxState.clickedI-ndex
        //     int taskIndex = wParam - 1000;

        //     if(taskIndex >= 0 && taskIndex < CustomListBoxState.tasks.size())
        //     {
        //         TaskRect &task = CustomListBoxState.tasks[taskIndex];
                
        //         // TODO: Comment this 
        //         bool needsUpdate = false;


        //         // // Calculate chars to color per tick based on length
        //         // int charsPerTick = 1;
        //         // int textLength = task.text.length();

                


        //         // if(textLength > 40) {
        //         //     charsPerTick = 30;
        //         // }
        //         // else if(textLength > 25) {
        //         //     charsPerTick = 15;
        //         // }
        //         // else if(textLength > 15) {
        //         //     charsPerTick = 10;
        //         // }


        //         // Calculate chars to color per tick based on length with max boundaries
        //         int charsPerTick = 1;
        //         int textLength = task.text.length();
        //          char debug[256];
        //         sprintf_s(debug, "Timer tick - Text length: %d, Current blueCount: %zu\n", 
        //                 textLength, task.blueCharCount);
        //         OutputDebugStringA(debug);



        //         // Ensure blueCharCount is valid
        //         if(task.blueCharCount > textLength)
        //         {
        //             task.blueCharCount = 0;  // Reset if invalid
        //         }

        //         if(textLength > 40 && textLength <= 100) {
        //             charsPerTick = 30;
        //         }
        //         else if(textLength > 25 && textLength <= 40) {
        //             charsPerTick = 15;
        //         }
        //         else if(textLength > 15 && textLength <= 25) {
        //             charsPerTick = 10;
        //         }

        //         charsPerTick = (charsPerTick <= textLength) ? charsPerTick : textLength;

        //         // Increment with boundary check 
        //         int newCount = task.blueCharCount + charsPerTick;
        //         DEBUG_BLUECHAR_CHANGE(task, "timer");
        //         task.blueCharCount = (newCount <= textLength) ? newCount : textLength;
        //         // task.blueCharCount = min(newCount, textLength);

        //         if(task.blueCharCount < textLength)
        //         {
        //             // Find the next word boundary
        //             size_t nextSpace = task.text.find_first_of(L" \t\r\n", task.blueCharCount);


        //              char debug[256];
        //             sprintf_s(debug, "Finding space: blueCount=%zu, textLength=%zu, nextSpace=%zu\n", 
        //                     task.blueCharCount, textLength, nextSpace);
        //             OutputDebugStringA(debug);


        //             if(nextSpace == std::wstring::npos) 
        //             {
        //                 task.blueCharCount = textLength;
        //             } 
        //             else 
        //             {
        //                 // Move to the start of the next word
        //                 // task.blueCharCount = nextSpace + 1;
        //                 size_t newPos = nextSpace + 1;
        //                 task.blueCharCount = (newPos <= textLength) ? newPos : textLength;
        //             }

        //             needsUpdate = true;
        //         }
        //         else
        //         {
        //             KillTimer(hwnd, task.timerId);
        //             task.timerId = 0;
        //             needsUpdate = true;
        //         }

        //         if(needsUpdate)
        //         {
        //             RECT itemRect;
        //             SendMessage(hwnd, LB_GETITEMRECT, taskIndex, (LPARAM)&itemRect);
        //             InvalidateRect(hwnd, &itemRect, FALSE);
        //         }
        //     }
        //     break;
        // }

        case WM_KILLFOCUS:
        {   
            // Reset current task if any is selected
            // We also need this code block (used in WM_LBUTTONDOWN) to reset the text color once we focus on the edit box or other client areas 
            if(CustomListBoxState.selectedIndex >= 0 && 
               CustomListBoxState.selectedIndex < CustomListBoxState.tasks.size())
            {
                // Handle the selected task animation cleaning/resetting        
                TaskRect& currentTask = CustomListBoxState.tasks[CustomListBoxState.selectedIndex];
                
                // If the current task has the Edit Control active, return early from here and let it go into edit mode. 
                if(currentTask.hTempEdit)
                {
                    return 0;
                }

                TaskFunctions::resetTaskAnimation(hwnd, &currentTask, false);

                // Within the same safety check, handle the clicked task's focus state
                // We know tasks vector is valid from outer check, just need valid index
                if(CustomListBoxState.clickedIndex >= 0 &&
                   CustomListBoxState.clickedIndex < CustomListBoxState.tasks.size())
                {
                    CustomListBoxState.tasks[CustomListBoxState.clickedIndex].hasFocus = false;
                }

                // Important: Reset editing state when focus is lost
                if(currentTask.isEditing)
                {
                    currentTask.isEditing = false;

                    // TODO: Probably there is no point saving tasks to file at this point, but check this better 
                    // // Save changes to storage
                    // try
                    // {
                    //     TaskStorage::saveTasksToFile(CustomListBoxState.tasks);
                    // }
                    // catch(const std::exception& e)
                    // {
                    //     std::cerr << "Failed to save edited task: " << e.what() << '\n';
                    // }
                }
            }

            // When list box loses focus reset selectedIndex to 'no selection' 
            CustomListBoxState.selectedIndex = -1;
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            clientRect.right = std::min((int)clientRect.right, 300);
            InvalidateRect(hwnd, &clientRect, FALSE);
            
            // InvalidateRect(hwnd, NULL, TRUE);
            break;
        }
    }


    return CallWindowProc(oldListBoxProc, hwnd, msg, wParam, lParam);
}

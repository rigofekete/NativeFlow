#include "task_functions.h"
#include "task_storage.h"

// extern FieldState CustomEditBoxState;


namespace HelperFunctions
{   
    // TODO: Lets call this function something else as it is not acting an the caret anymore
    // OR just get rid of it as it is only used in the calculateWrappedText  
    HDC setupCaretDC(HWND hwnd, HFONT *font)
    {
        HDC hdc = GetDC(hwnd);
        HFONT hFont = (HFONT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
        SelectObject(hdc, hFont);
        return hdc;
    }

    int CalculateTaskYPosition(const std::vector<TaskRect> *tasks, int taskIndex)
    {   
        int yPos = 0;
        // Sum up heights of all previous plus spacing
        for(int i = 0; i < taskIndex; ++i)
        {
        int height = tasks->at(i).bounds.top - tasks->at(i).bounds.top;
            yPos += height + taskYSpacing;
        }
        return yPos;
    }

    int HelperFunctions::calculateWrappedTextHeight(HWND hwnd, const std::wstring& text, const RECT& bounds)
    {
        // Get line count from edit control
        int lineCount = SendMessage(hwnd, EM_GETLINECOUNT, 0, 0);
        
        // // Debug output to confirm the line count of the current typed text 
        // wchar_t debug[512];
        // swprintf_s(debug, L"Line count: %d\n", lineCount);
        // OutputDebugString(debug);

        HFONT hFont;
        // Lets call this function something else as it is not acting an the caret anymore
        HDC hdc = setupCaretDC(hwnd, &hFont);
        
        // Get text metrics for line height
        TEXTMETRIC tm;
        GetTextMetrics(hdc, &tm);
        int lineHeight = tm.tmHeight;

        ReleaseDC(hwnd, hdc);

        // Calculate total height needed
        int totalHeight = lineHeight * lineCount;
        
        // Add padding
        totalHeight += (2 * taskYSpacing);

        return totalHeight;
    }

    LPARAM clickTaskXYPos(RECT* bounds)
    {
        // Calculate x using task bounds as before
        // IMPORTANT: Fist we need to get the actual task bound values because the ones coming from the LB_GETITEMRECT message return the task values with full width of the listbox. 
        int xPos = bounds->left + ((bounds->right - bounds->left) / 2);
        
        // Use task bounds for Y calculation too
        // IMPORTANT: We also need to get the actual task bound for the Y values since the LB_GETITEMRECT also returns the Y values with internal modifications which do not give us the actual area where the custom task bounds are....
        int yPos = bounds->top + ((bounds->bottom - bounds->top) / 2);
        
        LPARAM mouseParam = MAKELPARAM(xPos, yPos);

        return mouseParam;
    }

};


namespace ButtonFunctions
{
    void ButtonFunctions::addbutton(HWND hEditBox, HWND hListBox)
    { 
        if (!CustomEditBoxState.text.empty())
        {
            // Add to list box
            int newIndex = (int)SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)CustomEditBoxState.text.c_str());
            
            // Clear text from the edit box
            SetWindowText(hEditBox, L"");

            // Clear text from the struct member 
            CustomEditBoxState.text.clear();

            // Reset edit box to default size
            RECT currentRect;
            GetWindowRect(hEditBox, &currentRect);
            int currentWidth = currentRect.right - currentRect.left;
            
            // Reset to original height (taskHeight from globals.h)
            SetWindowPos(hEditBox, NULL, 
                        0, 0,               // Ignored due to SWP_NOMOVE
                        currentWidth, 
                        editBoxHeight,      // Use your default height constant
                        SWP_NOMOVE | SWP_NOZORDER);

            // // Get client rect and reset caret position
            // RECT clientRect;
            // GetClientRect(hEditBox, &clientRect);
            // SetCaretPos(clientRect.left + taskXPadding, 
            //             (clientRect.bottom - clientRect.top - caretHeight) / 2);
            
            InvalidateRect(hEditBox, NULL, TRUE);
            UpdateWindow(hEditBox);
        }
    }

    void delbutton(HWND hListBox)
    {   
        // Delete selected task if any
        if (CustomListBoxState.selectedIndex != -1)
        {
            // NOTE: In our custom listbox, we're not handling LB_GETCURSEL message - instead, we're tracking selection directly in CustomListBoxState.selectedIndex. That's why we need to use this value directly rather than asking the listbox for its selection.
            SendMessage(hListBox, LB_DELETESTRING, CustomListBoxState.selectedIndex, 0);
        }
    }

    void clrbutton(HWND hEditBox, HWND hListBox)
    {
        // TODO: Ask if user is sure about clearing all tasks with yes or no options 
        // TODO update: Actually I might just remove this button altogether  
        //
        // bool sure {false};
        // MessageBox(hListBox, TEXT("Are you sure you want to clear all tasks?"), NULL, MB_OK);

        // clear all tasks
        SendMessage(hListBox, LB_RESETCONTENT, 0, 0);
    }
}

namespace TaskFunctions
{
    void resetTaskAnimation(HWND hwnd, TaskRect *task, bool isKeyEscape)
    {
        if(task->timerId)
        {
            KillTimer(hwnd, task->timerId);
            task->timerId = 0;
        }

        // I decided to add this so that none of the values inside this block are reset when pressing escape key to exit the task editing
        if(!isKeyEscape)
        {
            // Always reset blue count and repaint (at the end of the case that calls this function). (For example we finish the painting and kill the timer for the previous task, timerID will be reset to 0 and that condition will not be called). We need to  reset blue count here and then force the repaint with the invalidate rect call at the end of whichever switch case calls this function 
            task->blueCharCount = 0;

            task->fullyBlue = false;
            // Clear task interaction states  
            task->isPressed = false;
            task->hasFocus = false;
            task->isHovered = false;
        }

        if(task->isEditing)
        {
            if(task->hTempEdit)
            {
                DestroyWindow(task->hTempEdit);
                task->hTempEdit = NULL;
                task->isEditing = false;
                // We need to do this to be able to repaint the task list box item in WM_DRAWITEM in main
                task->fullyBlue = false;
            }
        }
    }

    int TaskFunctions::findTaskUnderMouse(HWND hwnd, POINT mousePoint)
    {   
        if(CustomListBoxState.tasks.empty())
        {
            return -1; 
        }

        if(mousePoint.x < taskXPadding || mousePoint.x > maxTaskWidth)
        {
            return -1;
        }


        // Use list box's built-in hit testing
        // IMPORTANT: The key to get the actual task item with the scroll vscroll implementation is using Windows' built-in list box item position tracking (LB_ITEMFROMPOINT) instead of trying to track positions manually.
        // The key takeaway is that the code relies on the list box control's built-in item tracking functionality (LB_ITEMFROMPOINT and LB_GETITEMRECT) to determine the clicked task item based on the scroll position. This approach is more reliable than manually tracking item positions, as the list box control handles the scroll position and item coordinates internally.
        int hitTest = SendMessage(hwnd, LB_ITEMFROMPOINT, 0, 
                                MAKELPARAM(mousePoint.x, mousePoint.y));
        
        if(HIWORD(hitTest) == 0)
        {
            int index = LOWORD(hitTest);

            // Verify index is valid
            if(index >= 0 && index < CustomListBoxState.tasks.size())
            {
                

                // Get item rect to verify exact bounds
                RECT itemRect;
                // RECT itemRect = CustomListBoxState.tasks[index].bounds;
                SendMessage(hwnd, LB_GETITEMRECT, index, (LPARAM)&itemRect);

                // CustomListBoxState.tasks[index].bounds.right

                // Calculate actual task bounds within item rect
                RECT taskBounds = itemRect;
                taskBounds.left += taskXPadding;
                taskBounds.right = min(taskBounds.left + maxTaskWidth, CustomListBoxState.tasks[index].bounds.right);

                if(PtInRect(&taskBounds, mousePoint))
                {
                    return index;
                }
            }
        }
        return -1;
    }





};


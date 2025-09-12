// NOTE: No need to define Unicode like this beacuse it is set in the CmakeList.txt :)
// #define UNICODE
// #define _UNICODE 

// First, define this to prevent winsock.h from being included by windows.h
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

// List box styles
#define LBS_NOTIFY            0x0001L
#define LBS_SORT              0x0002L
#define LBS_OWNERDRAW         0x0020L
#define LBS_HASSTRINGS        0x0040L
#define LBS_USETABSTOPS       0x0080L
#define LBS_NOINTEGRALHEIGHT  0x0100L
#define LBS_MULTICOLUMN       0x0200L
#define LBS_WANTKEYBOARDINPUT 0x0400L  // Same value as LBS_WANTRETURN
#define LBS_EXTENDEDSEL       0x0800L
#define LBS_DISABLENOSCROLL   0x1000L
#define LBS_NODATA            0x2000L
#define LBS_NOSEL             0x4000L
#define LBS_COMBOBOX          0x8000L

#include "globals.h"
#include "task_storage.h"

#include <windows.h>
#include "subclasses.h"
#include "task_functions.h"
#include "frame_loader.h"
#include "resource.h"


#include <windows.h>
#include <string>

// Returns true if the system is Windows 11 (build >= 22000)
bool IsWindows11()
{
    HKEY hKey;
    // Open the registry key where Windows version info is stored.
    LONG lRes = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion",
        0,
        KEY_READ,
        &hKey);
    if (lRes != ERROR_SUCCESS)
    {
        return false; // Could not open key; assume Windows 10 or lower.
    }

    wchar_t buildNumber[128] = {0};
    DWORD bufferSize = sizeof(buildNumber);
    lRes = RegGetValueW(
        hKey,
        nullptr,
        L"CurrentBuild",
        RRF_RT_REG_SZ,
        nullptr,
        buildNumber,
        &bufferSize);

    RegCloseKey(hKey);

    if (lRes != ERROR_SUCCESS)
    {
        return false; // Reading error; assume Windows 10 or lower.
    }

    int build = _wtoi(buildNumber);
    return (build >= 22000);
}

// Use bool check returned from function above to check  ifthe system is Windows 11 (build >= 22000) and act accordingly.
////////////////////////////////////////////////////////////
void SetTitleBarAppearance(HWND window)
{
    if (!IsWindows11())  // Running on Windows 10 or lower.
    {
        BOOL darkMode = TRUE;
        HRESULT hr = DwmSetWindowAttribute(window, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));
        if (SUCCEEDED(hr))
        {
            // Successfully applied dark mode attribute on Windows 10 (if supported).
        }
        else
        {
            // Handle failure (or consider using a custom drawn title bar).
        }
    }
    else
    {
        // Windows 11-specific behavior (e.g., apply a custom caption color).
        COLORREF captionColor = RGB(0, 122, 204);
        DwmSetWindowAttribute(window, DWMWA_CAPTION_COLOR, &captionColor, sizeof(captionColor));
    }
}



// Define constants for control layouts
const int MARGIN = 20;              // Edge margin
const int CONTROL_SPACING = 10;     // Vertical spacing between controls
const int BUTTON_WIDTH = 100;
const int BUTTON_HEIGHT = 35;
const int EDITBOX_HEIGHT = 35;

internal win32_window_dimension Win32GetWindowDimension(HWND Window)
{
    win32_window_dimension Result;
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    Result.Width = ClientRect.right - ClientRect.left;
    Result.Height = ClientRect.bottom - ClientRect.top;

    return Result;
}


internal void RenderWeirdGradient(win32_onscreen_buffer *Buffer, int BlueOffset, int GreenOffset)
{
    uint8 *Row = (uint8 *)Buffer->Memory;
    for(int Y = 0; Y < Buffer->Height; ++Y)
    {
        uint32 *Pixel = (uint32 *)Row;
        for(int X = 0; X < Buffer->Width; ++X)
        {
            uint8 Blue = (X + BlueOffset);
            uint8 Green = (Y + GreenOffset);

            *Pixel++ = ((Green << 8) | Blue);
        }

        Row += Buffer->Pitch;
    }
}


// Original unmodified version 
// // Creating the backbuffer 
// internal void Win32ResizeDIBSection(win32_onscreen_buffer *Buffer, int Width, int Height)
// {
    
//     if(Buffer->Memory)
//     {
//         VirtualFree(Buffer->Memory, 0, MEM_RELEASE);
//     }

//     Buffer->Width = Width;
//     Buffer->Height = Height;
//     int BytesPerPixel = 4;

//     Buffer->Info.bmiHeader.biSize = sizeof(Buffer->Info.bmiHeader);
//     Buffer->Info.bmiHeader.biWidth = Buffer->Width;
//     Buffer->Info.bmiHeader.biHeight = -Buffer->Height;
//     Buffer->Info.bmiHeader.biPlanes = 1;
//     Buffer->Info.bmiHeader.biBitCount = 32;
//     Buffer->Info.bmiHeader.biCompression = BI_RGB;
//     // rest of the members are set to 0 through the static global declaration of BitmapInfo
    
//     int BitmapMemorySize = (Buffer->Width*Buffer->Height)*BytesPerPixel;
//     Buffer->Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);


//     Buffer->Pitch = Width*BytesPerPixel;

//     // // Debug message check 
//     // MessageBox(0, L"Win32ResizeDIBSection", L"Check", MB_OK);
// }

// internal void Win32UpdateWindow(win32_onscreen_buffer *Buffer, HDC DeviceContext, 
//                               int WindowWidth, int WindowHeight)

internal void UpdateWindowPlayFrame(HDC DeviceContext, 
                              int WindowWidth, int WindowHeight)
{
    // TODO: Add  !empty checks for all of the frame vectors 
    if(GlobalAnimation.IsPlaying && !GlobalAnimation.Frames.empty())
    {
        animation_frame *CurrentFrame = nullptr;

        if(GlobalAnimation.frameVector == L"Frames")
        {
            CurrentFrame = 
            &GlobalAnimation.Frames[GlobalAnimation.CurrentFrame];
        }
        if(GlobalAnimation.frameVector == L"FramesRed")
        {
            CurrentFrame = 
            &GlobalAnimation.FramesRed[GlobalAnimation.CurrentFrame];
        }
        if(GlobalAnimation.frameVector == L"FramesBlue")
        {
            CurrentFrame = 
            &GlobalAnimation.FramesBlue[GlobalAnimation.CurrentFrame];
        }

        // animation_frame *CurrentFrame = 
        //     &GlobalAnimation.Frames[GlobalAnimation.CurrentFrame];

        // Get listbox dimensions
        RECT listBoxRect;
        GetWindowRect(hListBox, &listBoxRect);
        POINT listBoxTopLeft = {listBoxRect.left, listBoxRect.top};
        POINT listBoxBottomRight = {listBoxRect.right, listBoxRect.bottom};
        ScreenToClient(GetParent(hListBox), &listBoxTopLeft);
        ScreenToClient(GetParent(hListBox), &listBoxBottomRight);

        // Position frame to the right of the list box with proper spacing
        int frameX = listBoxTopLeft.x + maxTaskWidth + (5 * MARGIN);
        int frameY = listBoxTopLeft.y;  // Align with top of listbox

        // Calculate available space considering listbox position
        int availableWidth = WindowWidth - frameX - MARGIN;
        int availableHeight = listBoxBottomRight.y - listBoxTopLeft.y;  // Match listbox height
        
        float scaleX = (float)availableWidth / CurrentFrame->Buffer.Width;
        float scaleY = (float)availableHeight / CurrentFrame->Buffer.Height;
        float scale = min(scaleX, scaleY);
        
        int scaledWidth = (int)(CurrentFrame->Buffer.Width * (scale - 0.3));
        int scaledHeight = (int)(CurrentFrame->Buffer.Height * (scale - 0.3));

        // Center vertically within available space if scaled height is less than available height
        if (scaledHeight < availableHeight) {
            frameY += (availableHeight - scaledHeight) / 2;
        }

        // Create clipping region to prevent drawing over listbox
        HRGN clipRegion = CreateRectRgn(
            frameX, frameY,
            frameX + scaledWidth,
            frameY + scaledHeight
        );
        SelectClipRgn(DeviceContext, clipRegion);

        // Draw the frame
        StretchDIBits(DeviceContext,
                      frameX, frameY, scaledWidth, scaledHeight,
                      0, 0, CurrentFrame->Buffer.Width, CurrentFrame->Buffer.Height,
                      CurrentFrame->Buffer.Memory,
                      &CurrentFrame->Buffer.Info,
                      DIB_RGB_COLORS, SRCCOPY);

        // Clean up
        SelectClipRgn(DeviceContext, NULL);
        DeleteObject(clipRegion);
    }
}

internal void UpdateWindowSingleFrame(HDC DeviceContext, 
                              int WindowWidth, int WindowHeight)
{
        if(!GlobalAnimation.Frames.empty())
        {
            animation_frame *CurrentFrame = 
                &GlobalAnimation.Frames[GlobalAnimation.CurrentFrame];

            // Get listbox dimensions
            RECT listBoxRect;
            GetWindowRect(hListBox, &listBoxRect);
            POINT listBoxTopLeft = {listBoxRect.left, listBoxRect.top};
            POINT listBoxBottomRight = {listBoxRect.right, listBoxRect.bottom};
            ScreenToClient(GetParent(hListBox), &listBoxTopLeft);
            ScreenToClient(GetParent(hListBox), &listBoxBottomRight);

            // Position frame to the right of the list box with proper spacing
            int frameX = listBoxTopLeft.x + maxTaskWidth + (2 * MARGIN);
            int frameY = listBoxTopLeft.y;  // Align with top of listbox

            // Calculate available space considering listbox position
            int availableWidth = WindowWidth - frameX - MARGIN;
            int availableHeight = listBoxBottomRight.y - listBoxTopLeft.y;  // Match listbox height
            
            float scaleX = (float)availableWidth / CurrentFrame->Buffer.Width;
            float scaleY = (float)availableHeight / CurrentFrame->Buffer.Height;
            float scale = min(scaleX, scaleY);
            
            int scaledWidth = (int)(CurrentFrame->Buffer.Width * (scale - 0.3));
            int scaledHeight = (int)(CurrentFrame->Buffer.Height * (scale - 0.3));

            // Center vertically within available space if scaled height is less than available height
            if (scaledHeight < availableHeight) {
                frameY += (availableHeight - scaledHeight) / 2;
            }

            // Create clipping region to prevent drawing over listbox
            HRGN clipRegion = CreateRectRgn(
                frameX, frameY,
                frameX + scaledWidth,
                frameY + scaledHeight
            );
            SelectClipRgn(DeviceContext, clipRegion);

            // Draw the frame
            StretchDIBits(DeviceContext,
                        frameX, frameY, scaledWidth, scaledHeight,
                        0, 0, CurrentFrame->Buffer.Width, CurrentFrame->Buffer.Height,
                        CurrentFrame->Buffer.Memory,
                        &CurrentFrame->Buffer.Info,
                        DIB_RGB_COLORS, SRCCOPY);

            // Clean up
            SelectClipRgn(DeviceContext, NULL);
            DeleteObject(clipRegion);
        }
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{


    // Older Networking Implementation //
    /////////////////////////////////////

    // // Initialize common controls
    // INITCOMMONCONTROLSEX icex;
    // icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    // icex.dwICC = ICC_WIN95_CLASSES;
    // InitCommonControlsEx(&icex);

    // ///////////////////////////////////////////////////////
    // ///////////////////////////////////////////////////////



    // // Convert command line to check for "server" or "client"
    // bool isServer = true; // Default to server 
    // if(strlen(lpCmdLine) > 0)
    // {
    //     std::string cmdLine(lpCmdLine);
    //     if(cmdLine.find("client") != std::string::npos)
    //     {
    //         isServer = false;
    //     }
    // }

    WNDCLASS wc {};

    // CS_HREDRAW|CS_VREDRAW flags that repaints the whole window after resizing (vertical and horizontal) and not just a portion of it 
    wc.style = CS_HREDRAW|CS_VREDRAW; 
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"TaskManagerWindow";
    // Set a custom Icon for our program with this member of wc 
    wc.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APPICON));
    // This wont't be needed now because we will deal with the background ourselves through the blitting method used by Casey 
    // wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);


    // Get the current screen resolution from the OS 
    int screenWidth  =  GetSystemMetrics(SM_CXSCREEN);
    int screenHeight =  GetSystemMetrics(SM_CYSCREEN);

   

    if(RegisterClass(&wc))
    {
        HWND window = CreateWindowEx(0,
                                    wc.lpszClassName,
                                    L"",
                                    // WS_CLIPCHILDREN needs to be ORed as a style to keep the child windows 
                                    // on top of the main background animation and painting loop
                                    // WS_OVERLAPPEDWINDOW|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
                                    // WS_OVERLAPPEDWINDOW includes resizing flag hence the need to use only WS_OVERLAPPED and manually select the titel bar buttons below
                                    // If you need to activate the resizing flags, uncomment the flag line above with the WS_OVERLAPPEDWINDOW flag combo 
                                    WS_OVERLAPPED|WS_CAPTION| WS_SYSMENU|WS_MINIMIZEBOX|WS_VISIBLE|WS_CLIPCHILDREN|WS_CLIPSIBLINGS,
                                    // Fixed coordinates without checking the current resolution on the system used 
                                    // 900, 20,                 // X and Y
                                    // Use the collected current resolution and center it on screen, regardless of the resoltuion used. 
                                    screenWidth / 3,             // Centered X
                                    screenHeight / 42,           // Centered Y
                                    800, 1200,                   // Width and Height
                                    0,
                                    0,
                                    hInstance,
                                    0);

        if(window)
        {
            
            // Safe check to resize window when Évi is using 150% dpi on her 1920x1080 monitor :)
            if(screenWidth < 2560 && screenHeight < 1440)
            {
                SetProcessDPIAware();            
            }


            ///////// Change the title bar appearance ///////
            ///////////////////////////////////////////////
    
            // // Set dark mode, or....
            ////////////////////////////
            // BOOL dark = TRUE;
            // DwmSetWindowAttribute(window, DWMWA_USE_IMMERSIVE_DARK_MODE, &dark, sizeof(dark));

            // // .. Set the custom color
            /////////////////////////////// 
            // COLORREF black = RGB(0,122,204);
            // DwmSetWindowAttribute(window, DWMWA_CAPTION_COLOR, &black, sizeof(COLORREF));

            // Optional //
            /////////////
            // // Make title bar transparent (if supported by Windows version)
            // DWMNCRENDERINGPOLICY ncrp = DWMNCRP_ENABLED;
            // DwmSetWindowAttribute(window, DWMWA_NCRENDERING_POLICY, &ncrp, sizeof(ncrp));

            // // Optional: Set caption color to transparent/black
            // COLORREF color = RGB(0, 0, 0);
            // DwmSetWindowAttribute(window, DWMWA_CAPTION_COLOR, &color, sizeof(COLORREF));


            // Different approach to set the title bar theme depending on the Windows version (win 10 or lesser or win 11)
            //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////      
            SetTitleBarAppearance(window);

            //////////////////////////////////////////////
            //////////////////////////////////////////////

  

            // Load animation frames
            GlobalAnimation.Frames = FrameLoader::LoadFrameSequence(L"frames");
            GlobalAnimation.FramesRed = FrameLoader::LoadFrameSequence(L"framesRed");
            GlobalAnimation.FramesBlue = FrameLoader::LoadFrameSequence(L"framesBlue");
            GlobalAnimation.CurrentFrame = 0;
            GlobalAnimation.frameVector = L"Frames";
            GlobalAnimation.IsPlaying = true;
            GlobalAnimation.TimerID = ANIMATION_START_TIMER;

            // If frames loaded successfully, start the timer
            if(!GlobalAnimation.Frames.empty() && GlobalAnimation.IsPlaying)
            {
                // SetTimer(window, GlobalAnimation.TimerID, 
                //         ANIMATION_FRAME_DURATION, NULL);
                SetTimer(window, GlobalAnimation.TimerID, 
                        70, NULL);
                
                SetTimer(window, ANIMATION_STOP_TIMER, 4450, NULL);
            }

            // Load saved tasks
            try
            {
                std::vector<TaskRect> savedTasks = TaskStorage::loadTasksFromFile();
            
                // At this stage the deadline data is still in memory loaded to the savedTasks, it will get erased after calling LB_ADDTSTRING. So, we are going to create a vector of taskDataPack struct objects to save the deadline data members we need so that after LB_ADDSTRING is called we can populate the new created TaskRects from that case (LB_ADDTRING) with the save deadline members (or any data member we want) we need to initialize/restore. 
                std::vector<taskDataPack> dataPack; 

                // Add each task to the listbox
                for(const TaskRect task : savedTasks)
                {
                    // store the data elements we need from the savedTasks to the taskDataPack struct 
                    dataPack.push_back(taskDataPack{task.data.deadline, task.data.activeDeadline, task.data.expired});
                    // Call ADDSTRING with the task text string (this call resets the taskrect task by creating a new task with the passed string. Thats why we wilkl populate the remaining deadline members in the next block with the saved data in the dataPack vector) 
                    SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)task.data.text.c_str());
                }    

                size_t index{};
   
                // After loading the tasks, sync TaskRect deadlines, restoring the deadline members from the dataPack
                for (TaskRect &task : CustomListBoxState.tasks) 
                {
                    if(dataPack[index].activeDeadline)
                    {
                        // Task is already reset by this time after calling the LB_ADDSTRING case a few lines above, which creates a new task only with the loaded string
                        // Because of this, and if the dataPack with the corresponding synced index has a true boolean for the activeDeadline flag, we proceed to populate the deadline members properly   
                        task.data.deadline = task.deadline = dataPack[index].deadline;
                        task.data.activeDeadline = dataPack[index].activeDeadline;
                        task.expired = dataPack[index].expired;
                        task.data.expired = dataPack[index].expired;
                        task.deadlineTimerID = index + 300;

                        // Start the timer for the tasks deadline 
                        SetTimer(window, task.deadlineTimerID, 1000, NULL);
                    }

                    index++;
                }

                // Give focus to the List Box first 
                SetFocus(hListBox);

                // Initialize scroll capabilities if we have enough tasks
                if (!savedTasks.empty())
                {
                    // Force initial scroll state setup
                    // IMPORTANT: We need to force this to make the vscroll bar handle available when starting the program (another quirk I had to override). 
                    SendMessage(hListBox, WM_VSCROLL, SB_TOP, 0);
                    SendMessage(hListBox, WM_VSCROLL, SB_BOTTOM, 0);
                    SendMessage(hListBox, WM_VSCROLL, SB_TOP, 0);
                    
                    // Make sure list box redraws properly
                    InvalidateRect(hListBox, NULL, TRUE);
                    UpdateWindow(hListBox);
                }
            }
            catch(const std::exception& e)
            {
                MessageBoxA(window, e.what(), "Error Loading Tasks", MB_OK | MB_ICONERROR);
            }
            
            HDC DeviceContext = GetDC(window);

            MSG msg {};

            // When starting program, set its focus to the default edit box control to keep things tidy and avoid flickering of the first task due to the internal Windows list box quirk behaviour.  
            SetFocus(hEditBox);

            GlobalRunning = true;
            while(GlobalRunning)
            {
                while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
                // while(GetMessage(&msg, window, 0, 0))
                {
                    if(msg.message == WM_QUIT)
                    {
                        GlobalRunning = false;
                    }
                    else
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);
                    }
                }

                // win32_window_dimension Dimension = Win32GetWindowDimension(window);
                // UpdateWindowSingleFrame(DeviceContext, Dimension.Width, Dimension.Height);
 
            }

            // Save the tasks vector right before closing the program to make sure all the members we want are saved properly (e.g. deadline) 
            TaskStorage::saveTasksToFile(CustomListBoxState.tasks);


        }
        else
        {
            MessageBox(NULL, L"Window creation failed!", L"Error", MB_ICONERROR);
            return 0;
        }
    }
    else
    {
        MessageBox(NULL, L"Window class registration failed!", L"Error", MB_ICONERROR);
        return 0;
    }

    return 0;
}


LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
        // Creating our Custom List Control (registered through the WinMain) 
        case WM_CREATE:
        {     
            

            hEditBox = CreateWindowEx(0,
                                    L"EDIT",
                                    NULL,
                                    WS_CHILD | WS_VISIBLE | ES_MULTILINE | 
                                    ES_AUTOVSCROLL | ES_WANTRETURN | ES_LEFT,  
                                    CW_USEDEFAULT, CW_USEDEFAULT, 
                                    CW_USEDEFAULT, CW_USEDEFAULT,
                                    hwnd,
                                    (HMENU)ID_EDITBOX,
                                    GetModuleHandle(NULL),
                                    NULL);

            // Subclass the edit control
            oldEditBoxProc = (WNDPROC)SetWindowLongPtr(hEditBox, GWLP_WNDPROC, (LONG_PTR)EditSubclassProc);  
            
            // WS_EX_TRANSPARENT is crucial here because:
            // Parent window paints (black background)
            // List box with WS_EX_TRANSPARENT waits for parent to finish
            // This allows your parent window's black background to show through if we prevent the list box background from being painted by the default procedure (with white bkgnd) by intercepting the CTLCOLORLIST and returning a NULL stock Brush to force it to paint nothing at all 
           // TODO: Check which style flags should I keep here  
            hListBox = CreateWindowEx(WS_EX_TRANSPARENT,
                                    L"LISTBOX",
                                    NULL, 
                                    WS_CHILD | WS_VISIBLE | LBS_NOTIFY | 
                                    // Owner-drawn list boxes specifically bypass the normal WM_PAINT handling to give you complete control over item appearance through WM_DRAWITEM. 
                                    LBS_OWNERDRAW | 
                                    // thses styles below are need for vertical scroll handle/wheel implementation 
                                    WS_VSCROLL | LBS_HASSTRINGS | LBS_DISABLENOSCROLL,  
                                    // LBS_NODATA, // Add this style to prevent default background
                                    CW_USEDEFAULT, CW_USEDEFAULT, 
                                    CW_USEDEFAULT, CW_USEDEFAULT,
                                    hwnd,
                                    (HMENU)ID_LISTBOX,
                                    GetModuleHandle(NULL),
                                    NULL);

            // After creating the List Box control, we can change the Windows theme for the Vscroll bar handle here: 
            if (hListBox)
            {
                // Initialize dark mode scrollbar
                BOOL darkMode = TRUE;
                DwmSetWindowAttribute(hListBox, DWMWA_USE_IMMERSIVE_DARK_MODE, &darkMode, sizeof(darkMode));
                
             
                // Modern dark style with blue accent
                SetWindowTheme(hListBox, L"DarkMode_Explorer", NULL);

                // Other VSCROLL HANDLE BAR styles  
                // // Light modern style with blue accent
                // SetWindowTheme(hListBox, L"Explorer", NULL);

                // // System style (takes system accent color)
                // SetWindowTheme(hListBox, L"ScrollBar", NULL);

                // // Classic Windows style
                // SetWindowTheme(hListBox, L"", NULL);

                // // Compact modern style
                // SetWindowTheme(hListBox, L"ItemsView", NULL);

                // // Another dark variant
                // SetWindowTheme(hListBox, L"DarkMode_ItemsView", NULL);
                
                // Force a visual update
                InvalidateRect(hListBox, NULL, TRUE);
                UpdateWindow(hListBox);
            }

            // Subclass the list control 
            oldListBoxProc = (WNDPROC)SetWindowLongPtr(hListBox, GWLP_WNDPROC, (LONG_PTR)ListSubclassProc);
            
            // Create the Add Task Bar Button
            hAddButton = CreateWindow(L"BUTTON",
                                      //   L"Add Task",
                                      L"ADD",
                                      // IMPORTANT NOTE: In order to avoid bugs in the new style defined in the button subclass
                                      // we need to set the BS_OWNERDRAW style flag. This will avoid  style interferences 
                                      // (like the bug locking the button with the default style look I was having when quickly clicking the button)
                                      WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                                      //   340, 240, 100, 30,
                                      CW_USEDEFAULT, CW_USEDEFAULT, 
                                      CW_USEDEFAULT, CW_USEDEFAULT,
                                      hwnd,
                                      (HMENU)ID_ADDBUTTON,
                                      GetModuleHandle(NULL), NULL);


            // Subclass Add Task button
            oldButtonProc = (WNDPROC)SetWindowLongPtr(hAddButton, GWLP_WNDPROC, (LONG_PTR)ButtonProc);
                    
            // Creat delete single task button
            hDeleteButton = CreateWindow(L"BUTTON",
                                        L"DEL",
                                        // IMPORTANT NOTE: In order to avoid bugs in the new style defined in the button subclass
                                        // we need to set the BS_OWNERDRAW style flag. This will avoid  style interferences 
                                        // (like the bug locking the button with the default style look  I was having when quickly clicking the button)
                                        WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                                        // 340, 280, 100, 30,
                                        CW_USEDEFAULT, CW_USEDEFAULT, 
                                        CW_USEDEFAULT, CW_USEDEFAULT,
                                        hwnd,
                                        (HMENU)ID_DELETEBUTTON,
                                        GetModuleHandle(NULL),
                                        NULL);

            // Subclass the Delete Tasks button
            oldButtonProc = (WNDPROC)SetWindowLongPtr(hDeleteButton, GWLP_WNDPROC, (LONG_PTR)ButtonProc);


            // Create Clear Tasks Button 
            hClrButton = CreateWindow(L"BUTTON",
                                    L"CLR ALL",
                                    // IMPORTANT NOTE: In order to avoid bugs in the new style defined in the button subclass
                                    // we need to set the BS_OWNERDRAW style flag. This will avoid  style interferences 
                                    // (like the bug locking the button with the default style look  I was having when quickly clicking the button)
                                    WS_CHILD | WS_VISIBLE | BS_OWNERDRAW,
                                    // 340, 320, 100, 30,
                                    CW_USEDEFAULT, CW_USEDEFAULT, 
                                    CW_USEDEFAULT, CW_USEDEFAULT,
                                    hwnd,
                                    (HMENU)ID_CLRBUTTON,
                                    GetModuleHandle(NULL), NULL);


            // Subclass the Clear Tasks button 
            oldButtonProc = (WNDPROC)SetWindowLongPtr(hClrButton, GWLP_WNDPROC, (LONG_PTR)ButtonProc);

            /// Text font ///
            /////////////////
            // Created font to be applied to our controls  
            hGlobalFont = CreateFont(16, 0, 0, 0, FW_LIGHT, FALSE, FALSE, FALSE,
                                     DEFAULT_CHARSET, OUT_OUTLINE_PRECIS, 
                                     CLIP_DEFAULT_PRECIS,
                                     CLEARTYPE_QUALITY, 
                                     VARIABLE_PITCH, 
                                     L"Liberation Mono");

            // Here we can pass the handles to the windows we want to apply the defined font in 
            SendMessage(hAddButton, WM_SETFONT, (WPARAM)hGlobalFont, TRUE);
            SendMessage(hClrButton, WM_SETFONT, (WPARAM)hGlobalFont, TRUE);
            SendMessage(hDeleteButton, WM_SETFONT, (WPARAM)hGlobalFont, TRUE);
            SendMessage(hListBox, WM_SETFONT, (WPARAM)hGlobalFont, TRUE);
            SendMessage(hEditBox, WM_SETFONT, (WPARAM)hGlobalFont, TRUE);


            // Smooth transition implementation 
            ///////////////////////////////////
            // Opacity increase when opening window 

            // Set window to be layered for transparency effect
            SetWindowLong(hwnd, GWL_EXSTYLE, GetWindowLong(hwnd, GWL_EXSTYLE) | WS_EX_LAYERED);
            // Set initial opacity to 0 (completely transparent)
            SetLayeredWindowAttributes(hwnd, 0, 0, LWA_ALPHA);
        
            // Set timer to trigger every 50ms
            SetTimer(hwnd, TRANSITION_TIMER, 50, NULL);

            break;
        }


        // Smooth transition case implementation 
        // Opacity increase when opening window 
        case WM_TIMER:
        {
            if(wParam == TRANSITION_TIMER)
            {
                // Start with transparent
                static int opacity = 0;
                // Increase opacity incrementation 
                opacity += 15;
                if(opacity >= 255)
                {
                    // Cap opacity
                    opacity = 255;
                    // Stop timer when fully opaque
                    KillTimer(hwnd, 1);
                }
                // Apply new opacity to the window
                SetLayeredWindowAttributes(hwnd, 0, (BYTE)opacity, LWA_ALPHA);
            }
            else if(wParam == ANIMATION_STOP_TIMER)
            {
                // Stop the animation
                if(GlobalAnimation.TimerID)
                {
                    KillTimer(hwnd, GlobalAnimation.TimerID);
                    GlobalAnimation.TimerID = ANIMATION_START_TIMER;  // Keep the original ID
                }
                GlobalAnimation.IsPlaying = false;
                KillTimer(hwnd, 3);
                
                // TODO: Check how to deal with these invalidations, probably we need to invalidate only the painted frame area 
                // // Force redraw to clear animation
                // InvalidateRect(hwnd, NULL, TRUE);
                // UpdateWindow(hwnd);
                // InvalidateRect(hListBox, NULL, TRUE);
                // UpdateWindow(hListBox);

                return 0;
            }
            // add && check for all the vectors with frames 
            else if(wParam == ANIMATION_START_TIMER && !GlobalAnimation.Frames.empty())
            {
                // GlobalAnimation.CurrentFrame = (GlobalAnimation.CurrentFrame + 1) % GlobalAnimation.Frames.size();

                // Get current window dimensions
                RECT clientRect;
                
                GetClientRect(hwnd, &clientRect);
                int WindowWidth = clientRect.right - clientRect.left;
                int WindowHeight = clientRect.bottom - clientRect.top;

                animation_frame* CurrentFrame = nullptr;

                if(GlobalAnimation.frameVector == L"Frames")
                {
                    GlobalAnimation.CurrentFrame = (GlobalAnimation.CurrentFrame + 1) % GlobalAnimation.Frames.size();
                    CurrentFrame = &GlobalAnimation.FramesRed[GlobalAnimation.CurrentFrame];
                }
                if(GlobalAnimation.frameVector == L"FramesRed")
                {
                    GlobalAnimation.CurrentFrame = (GlobalAnimation.CurrentFrame + 1) % GlobalAnimation.FramesRed.size();
                    CurrentFrame = &GlobalAnimation.FramesRed[GlobalAnimation.CurrentFrame];
                }
                if(GlobalAnimation.frameVector == L"FramesBlue")
                {
                    GlobalAnimation.CurrentFrame = (GlobalAnimation.CurrentFrame + 1) % GlobalAnimation.FramesBlue.size();
                    CurrentFrame = &GlobalAnimation.FramesBlue[GlobalAnimation.CurrentFrame];
                }

                
                // Calculate scaled dimensions (same logic as in Win32UpdateWindow)
                int frameX = MARGIN + maxTaskWidth + 20;
                int availableWidth = WindowWidth - frameX - MARGIN;
                int availableHeight = WindowHeight - (2 * MARGIN);
                
                float scaleX = (float)availableWidth / CurrentFrame->Buffer.Width;
                float scaleY = (float)availableHeight / CurrentFrame->Buffer.Height;
                float scale = min(scaleX, scaleY);
                
                int scaledWidth = (int)(CurrentFrame->Buffer.Width * (scale + 1));
                int scaledHeight = (int)(CurrentFrame->Buffer.Height * (scale + 1));

                // Calculate frame region using scaled dimensions
                RECT frameRegion;
                frameRegion.left = frameX;
                frameRegion.top = MARGIN + 20;
                // Adjust the right margin of the frame to avoid invalidating the scroll bar 
                // frameRegion.right = frameX + scaledWidth - 20;
                frameRegion.right = frameX + scaledWidth - 315;
                frameRegion.bottom = frameRegion.top + scaledHeight + 500;
                
                // Invalidate only the frame region
                InvalidateRect(hwnd, &frameRegion, FALSE);
            }
            
            auto now = std::chrono::system_clock::now();

            for(auto &task : CustomListBoxState.tasks)
            {
                if(task.deadlineTimerID == wParam && task.data.activeDeadline)
                {

                    if(task.deadline != std::chrono::time_point<std::chrono::system_clock>())
                    {
                        auto remaining = std::chrono::duration_cast<std::chrono::seconds>(task.deadline - now);

                        if(remaining.count() <= 0)
                        {
                            // Deadline expired
                            task.status = L"Deadline Expired";
                            task.expired = true;
                            task.data.expired = true;
                        }
                        else
                        {
                            // Update remaining time
                            std::wostringstream oss;
                            oss << L"Remaining: " << remaining.count() / 3600 << L"h "
                                << (remaining.count() % 3600) / 60 << L"m";
                            task.status = oss.str();
                        }
                    }

                    // // Refresh the list box
                    // InvalidateRect(hwnd, NULL, TRUE);
                }
            }

            break;
        }


        // NOTE Resizing of the main window is currently deactivated in the main creation styles, change back to WS_OVERLAPPEDWINDOW to activate it
        ////////////////////////////////////////////////////////////////////////////////////////
        case WM_SIZE:
        {
            // When resizing the main window, always check if any task state is either selected (fullyBlue) or in editing mode (pink) and reset them to default. 
            for(TaskRect &task : CustomListBoxState.tasks)
            {
                if(task.isEditing || task.fullyBlue)
                {
                    TaskFunctions::resetTaskAnimation(hwnd, &task, false);
                    break;
                }
            }   

            // Also reset selections and indexes  
            CustomListBoxState.selectedIndex = -1;
            CustomListBoxState.hoveredIndex = -1;
            CustomListBoxState.clickedIndex = -1;

            // Set focus to main window to decrease flickering of selected task 
            // (NOTE: when coming out of editing mode, from the special Edit Control, we are not able to override the internal Windows selection of that task. It is an internal Windows WinApi quirk that I could not bypass so far...) 
            SetFocus(hwnd);

            int width = LOWORD(lParam);
            int height = HIWORD(lParam);

            // Calculate total available height for list box
            int availableHeight = height - (3 * BUTTON_HEIGHT) - (3 * CONTROL_SPACING) - (3 * MARGIN);
            
            // Get number of items
            int numItems = SendMessage(hListBox, LB_GETCOUNT, 0, 0);
            
            // Calculate total content height including spacing
            int cumulativeHeight = 0;
            int completeItemsHeight = 0;
            int numVisibleItems = 0;

            for(int i = 0; i < numItems; i++) {
                int itemHeight = SendMessage(hListBox, LB_GETITEMHEIGHT, i, 0);
                
                // Check if adding this item would exceed available height
                if(cumulativeHeight + itemHeight <= availableHeight) {
                    completeItemsHeight = cumulativeHeight + itemHeight;
                    numVisibleItems++;
                }
                cumulativeHeight += itemHeight;
            }

            // Use the height that fits complete items
            int listBoxHeight = (numItems > 0) ? completeItemsHeight : availableHeight;
            listBoxHeight = max(listBoxHeight, taskHeight); // Ensure minimum height

            // Get current scroll info
            SCROLLINFO si = { sizeof(SCROLLINFO) };
            si.fMask = SIF_ALL;
            GetScrollInfo(hListBox, SB_VERT, &si);

            // Set up new scroll info if needed
            if(cumulativeHeight > availableHeight) {
                si.fMask = SIF_RANGE | SIF_PAGE;
                si.nMin = 0;
                si.nMax = cumulativeHeight;
                si.nPage = listBoxHeight;
                SetScrollInfo(hListBox, SB_VERT, &si, TRUE);
            }
                    
            // Moved to the start of the main file so thy can be used in the frame functions 
            // 
            // // Define constants for layout
            // const int MARGIN = 20;              // Edge margin
            // const int CONTROL_SPACING = 10;     // Vertical spacing between controls
            // const int BUTTON_WIDTH = 100;
            // const int BUTTON_HEIGHT = 35;
            // const int EDITBOX_HEIGHT = 35;

            // ListBox takes most of the space
            MoveWindow(hListBox,
                        MARGIN,                    // x: fixed left margin
                        MARGIN,                    // y: fixed top margin
                        width - (2 * MARGIN),      // width: window width minus margins
                        height - (3 * BUTTON_HEIGHT) - (3 * CONTROL_SPACING) - (3 * MARGIN), // height: remaining space
                        TRUE);

            // EditBox and buttons share a row
            int bottomControlsY = height - MARGIN - BUTTON_HEIGHT;
            
            MoveWindow(hEditBox,
                        MARGIN,                                                             // x: fixed left margin
                        bottomControlsY - (2 * BUTTON_HEIGHT) - (2 * CONTROL_SPACING),      // y: aligned with buttons
                        width - BUTTON_WIDTH - (3 * MARGIN),                                // width: remaining space minus button
                        EDITBOX_HEIGHT + 1,
                        TRUE);

            // Stack buttons vertically on the right
            MoveWindow(hAddButton,
                        width - MARGIN - BUTTON_WIDTH,    // x: right-aligned
                        bottomControlsY - (2 * BUTTON_HEIGHT) - (2 * CONTROL_SPACING),      // y: same as editbox
                        BUTTON_WIDTH,
                        BUTTON_HEIGHT,
                        TRUE);

            MoveWindow(hDeleteButton,
                        width - MARGIN - BUTTON_WIDTH,    // x: right-aligned
                        bottomControlsY - BUTTON_HEIGHT - CONTROL_SPACING,
                        BUTTON_WIDTH,
                        BUTTON_HEIGHT,
                        TRUE);

            MoveWindow(hClrButton,
                        width - MARGIN - BUTTON_WIDTH,    // x: right-aligned
                        bottomControlsY,
                        BUTTON_WIDTH,
                        BUTTON_HEIGHT,
                        TRUE);

            break;
        }       

        case WM_COMMAND:
        {   
            // // Block to check the line count on the edit box text and resize the rect height bound if necessary
            // //////////////////////////////////////////////////////////////////////////////////////////////////  

            // Get the notification code and control ID
            WORD notifyCode = HIWORD(wParam);
            WORD controlId = LOWORD(wParam);

            // Handle edit box notifications
            if (controlId == ID_EDITBOX && notifyCode == EN_CHANGE)
            {
                // Get current edit box text
                int textLength = GetWindowTextLength(hEditBox);
                std::vector<wchar_t> text(textLength + 1);
                GetWindowText(hEditBox, text.data(), textLength + 1);

                // Get current edit box rect
                RECT currentRect;
                GetWindowRect(hEditBox, &currentRect);
                MapWindowPoints(HWND_DESKTOP, hwnd, (LPPOINT)&currentRect, 2);

                // Calculate wrapped height
                int wrappedHeight = HelperFunctions::calculateWrappedTextHeight(hEditBox, 
                                                                                text.data(), 
                                                                                currentRect);

                // Add padding to the height
                // wrappedHeight += (2 * taskYSpacing);  // Using your global spacing constant

                // Set minimum height
                wrappedHeight = max(wrappedHeight, editBoxHeight);

                // Resize the edit box
                SetWindowPos(hEditBox, NULL,
                            currentRect.left,
                            currentRect.top,
                            currentRect.right - currentRect.left,
                            wrappedHeight,
                            SWP_NOMOVE | SWP_NOZORDER);

                // Force parent to redraw
                InvalidateRect(hwnd, &currentRect, TRUE);
                // UpdateWindow(hwnd);
            }
         
           
            // Decided to move each button click behaviour to a specific function to be called. It is less efficient then dealing with the button commands right here but to tidy up things a bit I will keep it like that for now 
            if (LOWORD(wParam) == ID_ADDBUTTON)
            {
                ButtonFunctions::addbutton(hEditBox, hListBox);
            }
            else if (LOWORD(wParam) == ID_DELETEBUTTON)
            {
                ButtonFunctions::delbutton(hListBox);
            }
            else if (LOWORD(wParam) == ID_CLRBUTTON) 
            {
                ButtonFunctions::clrbutton(hEditBox, hListBox);
            }
            break;
        }

     
        
  
  
        // // WM_CTLCOLOREDIT specifically deals with the text editing area - the "inner" part of the Edit control where text can be typed and displayed. It doesn't affect your custom rounded rectangle background or any other visual elements you've added through subclassing.
        // // This allows your parent window's black background to show through if we prevent the list box background from being painted by the default procedure (with white bkgnd) by intercepting this CTLCOLORLIST and return a NULL stock Brush to force it to paint nothing at all 
        case WM_CTLCOLOREDIT:
        {
            HDC hdc = (HDC)wParam;
            HWND hTempEdit = (HWND)lParam;

            // First check if this is for any of our task's temporary edit controls
            // Here we deal with the edit text region of our special Edit Control
            for(const auto& task : CustomListBoxState.tasks)
            {
                if(task.hTempEdit == hTempEdit && task.isEditing)
                {
                    SetBkMode(hdc, OPAQUE);  // Changed from TRANSPARENT
                    SetBkColor(hdc, RGB(40, 40, 40));  // Match task background color
                    SetTextColor(hdc, RGB(224, 29, 141));  // Pink for task editing
                    return (LRESULT)GetStockObject(NULL_BRUSH);

                    // TODO: Investigate tge advantages of doing this instead 
                    // // Create a static brush for better performance
                    // static HBRUSH hBrush = CreateSolidBrush(RGB(40, 40, 40));
                    // return (LRESULT)hBrush;  // Return actual background brush instead of NULL_BRUSH
                }
            }

            // If we get here it means no hTempEdit was found so it must be our main edit box (if hTempEdit i found wereturn early before reching here)
            if(hTempEdit == hEditBox)
            {
                SetBkMode(hdc, TRANSPARENT);
                SetTextColor(hdc, RGB(255, 255, 255));
                return (LRESULT)GetStockObject(NULL_BRUSH);
                // // TODO: Check why we define this static verion of the hNullBrush
                // static HBRUSH hNullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
                // return (LRESULT)hNullBrush;
            }

            return DefWindowProc(hwnd, msg, wParam, lParam);
        }

        // this case needs to be intercepted in order to avoid getting the list box background painted with default white color. We need to use this together with the control creation EX style WS_EX_TRANSPARENT defined for the List Box
        case WM_CTLCOLORLISTBOX:
        {
            HDC hdc = (HDC)wParam;
            SetBkMode(hdc, OPAQUE);
            SetBkColor(hdc, RGB(0, 0, 0));  // Match your window background
            // TODO: Investigate why is it worth it do make a static HBRUSH here
            static HBRUSH hBlackBrush = CreateSolidBrush(RGB(0, 0, 0));
            return (LRESULT)hBlackBrush;
        }

        // When we set the height in WM_MEASUREITEM, we're telling Windows "these are the valid drawing areas we need for our custom painting." By matching the heights:
        
        // Windows allocates the correct amount of space for each item
        // These become our "validated" drawing areas where we're allowed to paint
        // When we then draw our custom rectangles in WM_DRAWITEM, they fit perfectly within these validated areas
        // WM_MEASUREITEM is essentially us saying "these are the exact spaces we need validated for our custom drawing" and Windows respects that by allocating the correct areas for our WM_DRAWITEM painting.
        // Without matching these valid areas we will likely have black areas on top of our custom rects 
        case WM_MEASUREITEM:
        {
            MEASUREITEMSTRUCT* mis = (MEASUREITEMSTRUCT*)lParam;
            if (mis->CtlID == ID_LISTBOX) 
            {
                if (mis->itemID < CustomListBoxState.tasks.size()) 
                {
                    // Get the actual measured height + spacing (depending on wrapped text lines)
                    const TaskRect& task = CustomListBoxState.tasks[mis->itemID];
                    int itemHeight = task.bounds.bottom - task.bounds.top;
                    mis->itemHeight = itemHeight + taskYSpacing;
                }
                else 
                {
                    // Default height for new items
                    mis->itemHeight = taskHeight + taskYSpacing;
                }
                return TRUE;
            }
            break;
        }


    

        // TODO: Add relevant comments from older DRAWITEM case 
        case WM_DRAWITEM:
        {
            DRAWITEMSTRUCT* dis = (DRAWITEMSTRUCT*)lParam;

        
            if (dis->CtlID != ID_LISTBOX || dis->itemID == -1 ||
                dis->itemID >= CustomListBoxState.tasks.size())
                {
                    return FALSE;
                }
                
         
            HDC hdc = dis->hDC;
            RECT &rcItem = dis->rcItem;
            TaskRect& task = CustomListBoxState.tasks[dis->itemID];

            // Anti-Flickering apprach!
            ////////////////////////////
            // IMPORTANT: If this task is selected and fully blue, don't redraw its text at all and return early from this switch case in order to avoid unnecessary invalidations and repaitings which will cause fickeing on the tasks. Since selected task is already selected and fully painted in blue (confirmed by the fullyBlue boolean) we don't need to repaint it!
            // NOTE: We needed to add an extra check for the blue char == to text size to avoid occasional instances of this block getting called (maybe due to timer interference with the fullBue bool getting set) and not repainting the edit control area properly. 
            if(dis->itemID == CustomListBoxState.selectedIndex && task.blueCharCount == task.text.size() && task.fullyBlue)
            {
                return TRUE;
            }

            if(!task.isEditing) 
            {
                /////////////////////////////////////////////////
                // Get the client rect of the list box /////////
                ////////////////////////////////////////////////
                // CRUCIAL to do this to clean the remnants of the task painting when scrolling the list box with the wheel or the scroll handle 
                RECT listBoxRect;
                GetClientRect(hListBox, &listBoxRect);
                
                // Create an extended clear rect that includes the spacing between items
                RECT clearRect = dis->rcItem;
                clearRect.left = 0;
                // TODO: Test x > 300 fix 
                clearRect.right = min(maxTaskWidth, 300);
                // clearRect.right = maxTaskWidth;
                
                // Extend the clear area to include the spacing between items
                clearRect.top -= taskYSpacing;
            
                // Only clear rect with additional 4x extension if it is the very last task of the vector/listbox, this way we can be sure there wont be any remnants/artifacts at remaining empty space at the very bottom of the scrolled down list box  
                if (dis->itemID == CustomListBoxState.tasks.size() - 1) 
                {
                    clearRect.bottom = listBoxRect.bottom + 4*taskYSpacing;
                }
                        
                HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
                FillRect(dis->hDC, &clearRect, blackBrush);
                DeleteObject(blackBrush);

                /////////////////////////////////////////////
                /////////////////////////////////////////////
                // Task rect and border setup and painting // 

                rcItem.left += taskXPadding;
                
                SIZE textSize;
                GetTextExtentPoint32(hdc, task.text.c_str(), task.text.size(), &textSize);
                int currentWidth = (textSize.cx + 15) + (taskXPadding * 2); 
                // rcItem.right = (currentWidth > maxTaskWidth) ? maxTaskWidth : currentWidth;
                // TODO: Test x > 300 fix 
                rcItem.right = min((currentWidth > maxTaskWidth) ? maxTaskWidth : currentWidth, 300);


                RECT testRect = rcItem;
                testRect.left += taskXPadding;   
                testRect.right -= taskXPadding;  
                testRect.bottom = testRect.top + 1000;

                HFONT hFont = (HFONT)GetWindowLongPtr(hwnd, GWLP_USERDATA);
                SelectObject(hdc, hFont);

                TEXTMETRIC tm;
                GetTextMetrics(hdc, &tm);

                DrawText(hdc, task.text.c_str(), -1, &testRect, 
                        DT_WORDBREAK | DT_LEFT | DT_CALCRECT);

                int textHeight = testRect.bottom - testRect.top + tm.tmDescent;
                int totalHeight = max(textHeight + (2 * taskYSpacing), taskHeight);

                // TODO: Check better to learn it. VSCROLL related call. 
                SendMessage(hListBox, LB_SETITEMHEIGHT, dis->itemID, totalHeight + taskYSpacing);
                
                rcItem.bottom = rcItem.top + totalHeight;

                task.bounds = rcItem;
       
                HBRUSH bgBrush = CreateSolidBrush(RGB(40, 40, 40));
                HPEN nullPen = (HPEN)GetStockObject(NULL_PEN);
                SelectObject(hdc, nullPen);
                SelectObject(hdc, bgBrush);

                RoundRect(hdc, rcItem.left, rcItem.top,
                        rcItem.right, rcItem.bottom, 
                        taskXPadding, taskYSpacing);
                DeleteObject(bgBrush);

                if (task.isHovered)
                {
                    HBRUSH nullBrush = (HBRUSH)GetStockObject(NULL_BRUSH);
                    HPEN taskPen;

                    // if deadline expired paint a red border on hover status
                    if(task.expired)
                    {
                        taskPen = CreatePen(PS_SOLID, 2, RGB(255, 12, 33));
                    }
                    // else the standard blue 
                    else
                    {
                        taskPen = CreatePen(PS_SOLID, 2, RGB(0, 122, 204));
                    }
                    SelectObject(hdc, taskPen);
                    SelectObject(hdc, nullBrush);

                    RECT hoverRect = rcItem;
                    InflateRect(&hoverRect, -1, -1);
                    RoundRect(hdc, hoverRect.left, hoverRect.top,
                            hoverRect.right, hoverRect.bottom, 
                            taskXPadding, taskXPadding);

                    DeleteObject(taskPen);
                }
                
                if(!task.text.empty()) 
                {
                    SetBkMode(hdc, TRANSPARENT);
                    RECT textRect = task.bounds;
                    textRect.left += taskXPadding;
                    textRect.top += taskYSpacing;
                    textRect.right -= taskXPadding;


                    if(task.blueCharCount == task.text.size())
                    {
                        // If task deadline expired paint fully red state 
                        if(task.expired)
                        {
                            SetTextColor(hdc, RGB(255, 0, 0));
                        }
                        // else fully blue state
                        else
                        {
                            SetTextColor(hdc, RGB(0,153,255));
                        }
                        DrawText(hdc, task.text.c_str(), -1, &textRect,
                                DT_LEFT | DT_WORDBREAK);


                        // Anti-Flickering apprach!
                        ////////////////////////////
                        // Confirm that text is fully painted in blue by setting this bool to true and skip entering the painting logic of this DRAWITEM case in order to avoid flickering on the tasks due to unecessary repainting of the chars 
                        task.fullyBlue = true;
                    }
                    else if(task.blueCharCount > 0 && !task.expired)
                    {
                        
                        SetTextColor(hdc, RGB(255,255,255));
                        DrawText(hdc, task.text.c_str(), -1, &textRect,
                                DT_LEFT | DT_WORDBREAK);

                        // Find next word boundary using pre-calculated positions
                        //
                        // std::lower_bound returns an iterator to the first element that is not less than task.blueCharCount. Since wordBoundaries is a vector of size_t (storing the word ending positions), the iterator points to positions within this vector.
                        std::vector<size_t>::iterator it = std::lower_bound(task.wordBoundaries.begin(), 
                                                                            task.wordBoundaries.end(), 
                                                                            task.blueCharCount);

                        size_t wordEnd = (it != task.wordBoundaries.end()) ? *it : task.text.length();
                        
                        // draw blue portion
                        SetTextColor(hdc, RGB(0,122,204));
                        std::wstring blueText = task.text.substr(0, wordEnd);
                        DrawText(hdc, blueText.c_str(), -1, &textRect,
                                DT_LEFT | DT_WORDBREAK);
                    }
                    else
                    {
                        if(task.expired)
                        {
                            SetTextColor(hdc, RGB(191, 0, 0));
                        }
                        else
                        // Normal white text
                        {
                            SetTextColor(hdc, RGB(255,255,255));
                        }
                        DrawText(hdc, task.text.c_str(), -1, &textRect,
                                DT_LEFT | DT_WORDBREAK);
                    }
                }
            }
            return TRUE;
        }

    
        case WM_PAINT:
        {
            PAINTSTRUCT Paint;
            HDC DeviceContext = BeginPaint(hwnd, &Paint);
            
            wchar_t debugMsg[256];
            swprintf(debugMsg, 256, L"PAINT: rcPaint Left=%d, Right%d, Top=%d, Bottom%d\n",
                     Paint.rcPaint.left, Paint.rcPaint.right, Paint.rcPaint.top, Paint.rcPaint.bottom);
            OutputDebugString(debugMsg);  // See output in DebugView or Visual Studio's Output window

            // Paint black background
            RECT clientRect;
            GetClientRect(hwnd, &clientRect);
            HBRUSH blackBrush = CreateSolidBrush(RGB(0, 0, 0));
            FillRect(DeviceContext, &clientRect, blackBrush);
            DeleteObject(blackBrush);

            win32_window_dimension Dimension = Win32GetWindowDimension(hwnd);
            UpdateWindowPlayFrame(DeviceContext, Dimension.Width, Dimension.Height);
           
            EndPaint(hwnd, &Paint);
            break;
        }

        case WM_DESTROY:
        {

            // Kill animation timer
            if(GlobalAnimation.TimerID)
            {
                KillTimer(hwnd, GlobalAnimation.TimerID);
                GlobalAnimation.TimerID = 0;
            }

            // Clean up the frame buffer 
            GlobalAnimation.cleanup();

            // delete hFont handler for custom font here so it gets to last throughout the whole lifetime of the program scope (remember this font is being used by multiple controls). this way we make sure it gets painted at all times and also gets destroyed at the end of the scope.
            // Remember the font would get destroyed anyway at the end of scope but it is good practice to manage the memory allocation not only for good habit creation but also because this code could be used by other applications or DLL's (here we would have memory leak problems)
            DeleteObject(hGlobalFont); 
            PostQuitMessage(0);
            break;
        }

        case WM_ERASEBKGND:
        {
            return true;
        }

        default:
        {
            return DefWindowProc(hwnd, msg, wParam, lParam);
        }

    }
    
    return 0;
}

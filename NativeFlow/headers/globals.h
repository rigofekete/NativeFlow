#pragma once

// TODO: Understand what this is in depth 
// First, define this to prevent winsock.h from being included by windows.h
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif


// these ones below are the Boost library includes 
#include <boost/serialization/string.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>

#include <SDKDDKVer.h>  // This one is needed for the subclassing of the Edit Ctrl fot the List Box
#include <algorithm>
#include "windows.h"
// We need to add these to control the Windows themes so we can chnge and set a darker one for our List Box 
// IMPORTANT: we need to set these after the WIN#@_LEAN_AND_MEAN and windows.h
#include <uxtheme.h>
#include <dwmapi.h>
#pragma comment(lib, "uxtheme.lib")
#pragma comment(lib, "dwmapi.lib")

#ifndef DWMWA_USE_IMMERSIVE_DARK_MODE
#define DWMWA_USE_IMMERSIVE_DARK_MODE 20
#endif

#ifndef DWMWA_CAPTION_COLOR
#define DWMWA_CAPTION_COLOR 35
#endif

#include <windowsx.h>
#include <commctrl.h> // This one is needed for the subclassing of the Edit Ctrl for the List Box
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
//  stdint (or cstdint is the C++ header that provides fixed-width integer types like uint8_t, uint16_t, etc. Without it, these types aren't defined.
#include <stdint.h> 
#include <chrono>
#include "buffer_types.h"
#include "frame_loader.h"

// TODO: Study the differences between a regular subclass (like the List Box, Edit Box) and this special subclass (special Edit Control)
// We need these in order to subclass an Edit Control for the List Box text editing handling 
// Link with ComCtl32.lib
#pragma comment(lib, "comctl32.lib")

// Manifest for Common Controls v6.0
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

// Constants for the control IDS
#define ID_LISTBOX 101
#define ID_ADDBUTTON 102 
#define ID_CLRBUTTON 103
#define ID_EDITBOX 104
#define ID_DELETEBUTTON 105
#define ID_MAIN 106

// Timer ID defines //
//////////////////////

#define TRANSITION_TIMER 1
#define ANIMATION_START_TIMER 2
#define ANIMATION_STOP_TIMER 3
#define AMIMATION_SINGLE_TIMER 4

// Frame duration 
#define ANIMATION_FRAME_DURATION 80

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

// Defined macros to be identified as static
// Casey named these macros according to what they are doing in the code more clearly, 
// this way we can have specific keywords for differrent types of static purposes, since they can be different as you can read in the description of each one of them below
//
// To be used with functions only, means that the function is local only to the source file it is in   
#define internal static
// means the variable is locally peristant, like the Operation that contains WHITENESS/BLACKNESS. 
// We want that to keep the last value assigned until if else/logic changes it  
#define local_persist static
// simply means that the variable is global 
#define global_variable static
////////////// IMPORTANT NOTE ////////////////////
// Addtional notes on the usage of static ///////
/////////////////////////////////////////////////
// Static is used here instead of simply declaring the variables and functions as global 
// because static can also act as a global with the plus of nothing being able to use this name outside this 'translation unit'  
// in other words, means file or all files included by this file as well. 
//
// Static variables when declared without initialization are also automatically initialized to 0.
//
// Static used with functions also prevents the function being called by name in other translation units 

// Function prototypes 
LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

////// Globals /////

// Declared as const global because they are fixed measurement values (for tasks in the list box and edit box) that wont be changed anywhere in the program 
global_variable int editBoxHeight = 35;

global_variable int taskHeight   = 35;    // Task rect height
global_variable int taskYSpacing = 10;    // Task rect y spacing/padding
global_variable int taskXPadding = 10;    // task rect x spacing/padding
global_variable int maxTaskWidth = 250 + (taskXPadding * 2);   // maximum width possible for a task rect 

// (definition in globals.cpp)
extern bool GlobalRunning;


// Set the last key pressed so the KILLFOCUS handler of the Special Edit Control proc (TempEditSubclassProc) can check it do the operations accordingly  
// (definition in globals.cpp)
extern WPARAM lastKeyPressed;  

// extrn globals to store the created child window handles from WM_CREATE
// (definition in globals.cpp)
extern HWND hListBox, hAddButton, hClrButton, hEditBox, hDeleteButton;

// extern global font variable to receive new created font handle
// (definition in globals.cpp)
extern HFONT hGlobalFont;

// extern Global variable to store the original EditBox and Button procedure 
// (definition in globals.cpp)
extern WNDPROC oldEditBoxProc;
extern WNDPROC oldListBoxProc;
extern WNDPROC oldButtonProc;

// IMPORTANT: This had to be moved to the buffer_types.h in order to define the animation_frame struct
//
// struct win32_onscreen_buffer
// {
//     // Pixels are always 32-bits wide, LE Memory Order BB GG RR XX
//     BITMAPINFO Info;
//     void *Memory;
//     int Width;
//     int Height;
//     int Pitch;
// };

// This remains here though....
// declaring extern global struct object to be used across the main funtions 
extern win32_onscreen_buffer GlobalBackbuffer;



struct win32_window_dimension
{
    int Width;
    int Height; 
};


struct taskDataPack
{
    std::chrono::system_clock::time_point deadline;
    bool activeDeadline {};
    bool expired {};
    // TODO
    // Maybe we can add other members here later in case we need to initialize them 
};


// BOOST implementation struct
// For network serialization of just the task data 
struct TaskData
{
    // TODO: Implement versioning 
    int version = 1;            // initial version
    std::wstring text{};
    std::chrono::time_point<std::chrono::system_clock> deadline{};
    bool activeDeadline{};
    bool expired {};
    
    // We might want to add:
    // time_t timestamp;        // When task was created/modified
    // std:wstring creator      // Who created the task

    private:
        // For serialization
        // boost::serialization::access is a special class in the Boost.Serialization library that acts as a gatekeeper for the serialization process. By declaring it as a friend of this struct, you're giving this class permission to access private members of your TaskData struct during serialization and deserialization (the serialize() template function is a member of the access class)     
        friend class boost::serialization::access;

        // template<class Archive> means this function can work with different types of archives:
        //
        // Text archives (save as readable text)
        // Binary archives (save as binary data)
        // XML archives
        // etc.
        template<class Archive>
        // VERSIONING NOTE* (notes at the end of the file)
        void serialize(Archive &ar, const unsigned int version)
        {
            // The ar & text part is using an overloaded operator that means both save AND load:
            // When saving: ar & data means "put data INTO the archive"
            // When loading: ar & data means "get data FROM the archive"
            // ar & text;
            ar & boost::serialization::make_nvp("text", text);


            // The boost::serialization::make_nvp function is an essential part of Boost.Serialization, a popular C++ library for serializing and deserializing complex data structures. The purpose of make_nvp (Name-Value Pair) is to provide human-readable names for the serialized fields. Serialized data often needs to be readable by humans, especially in text-based formats like XML or JSON. Using NVPs makes the output more understandable.
            ar & boost::serialization::make_nvp("expired", expired);

            // Save active deadline flag as well so we can keep track of which tasks need timer when loading data after we open the program 
            ar & boost::serialization::make_nvp("active_deadline", activeDeadline);

            // TODO: This method apparently makes the deadline value lose precision and making the deadlines evetually get lost randomly when opening the program. Check if this is truly the case
            /////////////////////////////////////////////////////// 
            // // Convert deadline from time_point to time_t while saving and vice versa for loading (apparently time_t is not a type which can be serialized - need to investigate this though)
            // ///////////////////////////////////////////////////////////////////////////////////////////
            // // Serialize the deadline as time_t (convert time_point to time_t, since Boost.Serialization does not directly support the std::chrono::time_point type)
            // time_t deadlineTime; 
            // if(Archive::is_saving::value)
            // {
            //     deadlineTime = std::chrono::system_clock::to_time_t(deadline);
            // }

            // // Using boost::serialization::make_nvp provides readable names for XML archives. It's optional but useful for debugging and versioning.
            // ar & boost::serialization::make_nvp("deadline", deadlineTime);
            
            // // When loading, convert time_t back to time_point
            // if(Archive::is_loading::value)
            // {
            //     deadline = std::chrono::system_clock::from_time_t(deadlineTime);
            // }


            // // NOTE: Second method attempt to make time_point conversion (this time using deadline.time_since_epoch().count(); ) to properly save the data wihout losing precision
            // ////////////////////////////////////////////////////
            // // Save deadline as its duration count (integer value)
            // auto deadlineCount = deadline.time_since_epoch().count();
            // ar & boost::serialization::make_nvp("deadline", deadlineCount);

            // // When loading, reconstruct the time_point
            // if(Archive::is_loading::value)
            // {
            //     deadline = std::chrono::system_clock::time_point(std::chrono::system_clock::duration(deadlineCount));
            // }


            // // Third method attempt to save time_point deadline data
            // /////////////////////////////////////////////////////////
            // // Convert time_point to an ISO 8601 string
            // std::wstring deadlineStr;
            // if (Archive::is_saving::value)
            // {
            //     std::time_t deadlineTime = std::chrono::system_clock::to_time_t(deadline);
            //     std::wstringstream ss;
            //     ss << std::put_time(std::gmtime(&deadlineTime), L"%Y-%m-%dT%H:%M:%S");
            //     deadlineStr = ss.str();
            // }

            // ar & boost::serialization::make_nvp("deadline", deadlineStr);



            // if (Archive::is_loading::value)
            // {
            //     std::tm tm = {};
            //     std::wistringstream ss(deadlineStr);
            //     ss >> std::get_time(&tm, L"%Y-%m-%dT%H:%M:%S");

            //     if (!ss.fail()) // Ensure parsing succeeded
            //     {
            //         // Convert from UTC to a time_t value
            //         std::time_t time = _mkgmtime(&tm); // Windows equivalent of timegm
            //         deadline = std::chrono::system_clock::from_time_t(time);
            //     }
            //     else
            //     {
            //         deadline = std::chrono::system_clock::time_point(); // Set to epoch if parsing fails
            //     }
            // }

            // Forth method 
            ////////////
            // Store time_point as an integer (seconds since epoch)
            std::int64_t deadlineSeconds = 0;

            if (Archive::is_saving::value)
            {
                deadlineSeconds = std::chrono::duration_cast<std::chrono::seconds>(
                    deadline.time_since_epoch()).count();
            }

            ar & boost::serialization::make_nvp("deadline", deadlineSeconds);

            if (Archive::is_loading::value)
            {
                deadline = std::chrono::system_clock::time_point(std::chrono::seconds(deadlineSeconds));
            }

        }

};

///////////////////////////
////// Frame structs //////
///////////////////////////

struct animation_state
{
    std::vector<animation_frame> Frames;
    std::vector<animation_frame> FramesRed;
    std::vector<animation_frame> FramesBlue;
    size_t CurrentFrame;
    std::wstring frameVector;
    bool IsPlaying;
    UINT_PTR TimerID;


    void cleanup()
    {
        // Stop animation
        if(TimerID)
        {
            KillTimer(NULL, TimerID);
            TimerID = 0;
        }

        // use existing cleanup function for each frame
        for(animation_frame &frame : Frames)
        {
            FrameLoader::CleanupFrameBuffer(&frame.Buffer);
        }
        // use existing cleanup function for each frame
        for(animation_frame &frame : FramesRed)
        {
            FrameLoader::CleanupFrameBuffer(&frame.Buffer);
        }
        // use existing cleanup function for each frame
        for(animation_frame &frame : FramesBlue)
        {
            FrameLoader::CleanupFrameBuffer(&frame.Buffer);
        }
        Frames.clear();
        FramesRed.clear();
        FramesBlue.clear();
    }

};

// Add to external declarations
// (definition in globals.cpp)
extern animation_state GlobalAnimation;

struct render_state 
{
    bool ShowBackground;
    bool ShowAnimation;
    float AnimationAlpha;
};

extern render_state GlobalRenderState;


///////////////////////////
////// Other structs //////
/////////////////////////// 

struct TaskRect
{
    TaskData data;          // The actual task data that will be synced with Boost 

    // UI-specific members 
    RECT bounds;
    std::wstring text;
    bool isHovered    = false;
    bool isPressed    = false;
    bool hasFocus     = false;
    bool isEditing    = false;
    bool fullyBlue    = false;
    
    // handle for the special text edit control (TempEditSubclassProc)
    HWND hTempEdit    = NULL; 

    // Deadline timer implementation 
    /////////////////////////////////

    // Deadline time
    std::chrono::time_point<std::chrono::system_clock> deadline;
    // Remaining time or status like "Deadline Expired"
    std::wstring status;
    bool expired = false;
    UINT_PTR deadlineTimerID = 0;

    // BLue char animation members 
    //////////////////////////////


    size_t blueCharCount = 0;        // track colored characters 
    UINT_PTR timerId  = 0;        // Store timer ID



    // Implementation to find word boundaries in order to incremente and paint blue chars when selecting a task.
    ////////////////////////////////////////////////////////////////////////////////////////////////////////// 
    
    // Store word end positions
    std::vector<size_t> wordBoundaries;  


    // This function will be called to calculate word boundaries in LBUTTONDOWN in Lisbox proc, right before staring the timer  
    void calculateWordBoundaries() 
    {
        // Find positions of the word boundaries and store them in the vector of a size_t boundaries 
        wordBoundaries.clear();
        size_t pos = 0;
        while((pos = text.find_first_of(L" \t\r\n", pos)) != std::wstring::npos) 
        {
            wordBoundaries.push_back(pos);
            pos++;
        }
        // Add the end of text as final boundary
        wordBoundaries.push_back(text.length());
    }

    //////////////////////////////////////////////////////////////////////////////////////////////////////////////

    // Boost Serialization to store and load data 
    ////////////////////////////////////////////// 

    private:
        friend class boost::serialization::access;
        
        template<class Archive>
        // VERSIONING NOTE* (notes at the end of the file)
        void serialize(Archive & ar, const unsigned int version)
        {
            // The ar & data part is using an overloaded operator that means both save AND load:
            // When saving: ar & data means "put data INTO the archive"
            // When loading: ar & data means "get data FROM the archive"
            ar & data;        // Serialize the TaskData part
            
            // TODO: I thibk this is not necessary, we really only need to save and load the task.data, thats why it is there for anyway
            //
            // if(Archive::is_loading::value)
            // {
            //     // Sync the TaskRect deadline with TaskData's deadline
            //     deadline = data.deadline;

            //     // Optionally, update the status
            //     if(deadline != std::chrono::system_clock::from_time_t(0))
            //     {
            //         auto now = std::chrono::system_clock::now();
            //         auto remaining = std::chrono::duration_cast<std::chrono::seconds>(deadline - now);

            //         if(remaining.count() <= 0)
            //         {
            //             status = L"Deadline Expired";
            //         }
            //         else
            //         {
            //             std::wostringstream oss;
            //             oss << L"Remaining: " << remaining.count() / 3600 << L"h " 
            //                 << (remaining.count() % 3600) / 60 << L"m";
                        
            //             status = oss.str(); 
            //         }
            //     } 
            // }



            // We don't serialize UI-specific members as they're reconstructed on load
        }
};


// TODO: Put these externs encapsulated in a namespace for better safety
// Global struct with field states (button, editbox, listbox, etc.) booleans and additional data for specific controls  
struct ButtonBoxState
{
    bool isHovered;
    bool isPressed;
    bool hasFocus;
    bool isFormatted = false;

    std::wstring text; // to store the edit text
    // int caretPos;    // Task caret position 
};

// TODO: Put these externs encapsulated in a namespace for better safety
// declaring extern global struct object to be used across the main funtions 
// (definition in globals.cpp)
extern ButtonBoxState CustomEditBoxState;

struct ListBoxState
{
    std::vector<TaskRect> tasks;
    int clickedIndex  = -1;      // set all of these to -1 (no selection)
    int hoveredIndex  = -1;
    int selectedIndex = -1;
};

// (definition in globals.cpp)
extern ListBoxState CustomListBoxState;


// (definition in globals.cpp)
extern int XOffset;
extern int YOffset;




// Debug function extern declaration (definition in globals.cpp)
// Leaving these here for documentation and examplification of how to use similar debugging techniques.  
extern void DebugTaskState(const TaskRect& task, const char *location);

#define DEBUG_BLUECHAR_CHANGE(task, location) \
    { char debug[256]; \
      sprintf_s(debug, "BlueChar changed at %s: %zu -> %zu\n", \
                location, task.blueCharCount, newCount); \
      OutputDebugStringA(debug); }


// *VERSIONING NOTE (About BOOST):
// The 'version' parameter in serialize() helps handle files saved by different versions
// of the program. For example:
//
// Version 1: struct has only 'text' field
// Version 2: struct adds 'creationDate' field
//
// serialize() checks version and knows:
// - For version 1 files: only load 'text'
// - For version 2 files: load both 'text' and 'creationDate'
//
// This way newer versions of the program can still read old files,
// and we can add new fields to TaskData in the fu

// NOTE: No need to define Unicode like this beacuse it is set in the CmakeList.txt :)
// #define UNICODE
// #define _UNICODE 

#pragma once

#include "globals.h"
#include "task_functions.h"
#include "task_storage.h"
#include "dialog.h"
#include <sstream>
// #include "network_manager.h"

LRESULT CALLBACK ButtonProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK EditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ListSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK TempEditSubclassProc(HWND hwnd, UINT msg, WPARAM wParam, 
                                     LPARAM lParam, UINT_PTR uIdSubclass, 
                                     DWORD_PTR dwRefData);




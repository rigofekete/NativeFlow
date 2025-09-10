#pragma once

#include <windows.h>
#include <globals.h>

// Resource IDs
#define IDD_DARK_DIALOG  201
#define IDD_COUNT_DIALOG 202
// Urgent 
#define IDBUTTON1        1
// !!IMPORTANT!! ID 2 seems reserved for the close button of the diaog box. I chose not to have any button to close the Dialog, this will be assigned to the ESC or close window X 
#define IDBUTTON2        2
// Free / reset
#define IDBUTTON3        3
//  Soon 
#define IDBUTTON4        4
#define IDC_STATIC       -1

// // externs for the dialog button handles 
// extern HWND hwndButton1, hwndButton2;

// Function to show the dark-theme dialog 
void ShowDarkDialog(HWND hwndParent);
void ShowCountDialog(HWND hwndParent);



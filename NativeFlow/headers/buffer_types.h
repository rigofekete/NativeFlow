#pragma once

#include <windows.h>

struct win32_onscreen_buffer
{
    BITMAPINFO Info;
    void *Memory;
    int Width;
    int Height;
    int Pitch;
};


struct animation_frame
{
    win32_onscreen_buffer Buffer;
    int DurationMS;
};

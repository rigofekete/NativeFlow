#pragma once 

#include <vector>
#include <string>
#include "buffer_types.h"

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

// forward declaration so we can make it friend of the FrameLoader class 
struct animation_states;

class FrameLoader
{
    // Give animation_state access to private members
    friend struct animation_state;

    public:
        static std::vector<animation_frame> LoadFrameSequence(const std::wstring &directoryPath);
        static bool LoadSingleFrame(const std::wstring & flepath, animation_frame *outFrame);

    private:
        static win32_onscreen_buffer CreateFrameBuffer(int width, int height);
        static void CleanupFrameBuffer(win32_onscreen_buffer *buffer);
};

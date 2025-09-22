#include "frame_loader.h"
// #include <windows.h>
#include <objidl.h>      // for gdi+
#include <gdiplus.h>     // for gdi+
#include <filesystem>
#pragma comment(lib, "gdiplus.lib");  // for gdi+

namespace fs = std::filesystem;

std::vector<animation_frame> FrameLoader::LoadFrameSequence(const std::wstring &directoryPath)
{
    std::vector<animation_frame> frames;

    for(const auto &entry : fs::directory_iterator(directoryPath))
    {
        if(entry.path().extension() == L".png")
        {
            animation_frame frame = {};
            if(LoadSingleFrame(entry.path().wstring(), &frame))
            {
                // Defaut duration that each frame remains painted on the screen (in milliseconds)
                frame.DurationMS = 170;  
                frames.push_back(frame);
            }
        }
    }

    return frames;
}

bool FrameLoader::LoadSingleFrame(const std::wstring &filepath, animation_frame *outFrame)
{
    // Initialize GDI +
    ///////////////////
    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    Gdiplus::GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    // Load the bitmap
    //////////////////
    Gdiplus::Bitmap *bitmap = Gdiplus::Bitmap::FromFile(filepath.c_str());
    if(!bitmap || bitmap->GetLastStatus() != Gdiplus::Ok)
    {
        Gdiplus::GdiplusShutdown(gdiplusToken);
        return false;
    }

    // Create the bitmap pixels
    outFrame->Buffer = CreateFrameBuffer(bitmap->GetWidth(), bitmap->GetHeight());
    
    // Lock the bitmap pixels
    Gdiplus::BitmapData bitmapData;
    Gdiplus::Rect rect(0, 0, bitmap->GetWidth(), bitmap->GetHeight());
    bitmap->LockBits(&rect, Gdiplus::ImageLockModeRead, PixelFormat32bppARGB, &bitmapData);


    // Copy pixel data
    uint8 *srcRow = (uint8 *)bitmapData.Scan0;
    uint8 *destRow = (uint8 *)outFrame->Buffer.Memory;

    for(int y = 0; y < bitmap->GetHeight(); y++)
    {
        // Convert ARGB to your buffer format (which is XRGB) 
        uint32 *srcPixel = (uint32 *)srcRow;
        uint32 *destPixel = (uint32 *)destRow;

        for(int x = 0; x < bitmap->GetWidth(); x++)
        {
            // Get ARGB values
            uint32 pixel = *srcPixel++;
            uint8 alpha  = (pixel >> 24) & 0xFF;
            uint8 red    = (pixel >> 16) & 0xFF;
            uint8 green  = (pixel >> 8) & 0xFF;
            uint8 blue   = pixel & 0xFF;

    
             // TODO: Study in depth why we need to do these operation to display the image transparency 
            //  // Pre-multiply RGB values with alpha to handle image transparency properly
            red = (red * alpha) / 255;
            green = (green * alpha) / 255;
            blue = (blue * alpha) / 255;

            // Store in buffer format (XRGB or XBGR depending on your needs)
            ////////////////////////////////////////////////////////////////

            // NOT CORRECTLY DEALING WITH THE ALPHA CHANNEL WHICH WAS CAUSING THE PROBLEM DISPLAYING TRANSPARENCY FOR THE PNG FRAME
            // *destPixel++ = (red << 16) | (green << 8) | blue;
            
            // This plus change plus the Pre-multiply RGB code block above are necessary to display the transparency from the png frame.   
            *destPixel++ = (alpha << 24) | (red << 16) | (green << 8) | blue; 
        }

        srcRow += bitmapData.Stride;
        destRow += outFrame->Buffer.Pitch;
    }
    
    // Cleanup
    bitmap->UnlockBits(&bitmapData);
    delete bitmap;
    Gdiplus::GdiplusShutdown(gdiplusToken);

    return true;
}


win32_onscreen_buffer FrameLoader::CreateFrameBuffer(int width, int height)
{
    win32_onscreen_buffer buffer = {};

    buffer.Width = width;
    buffer.Height = height;
    int BytesPerPixel = 4;

    buffer.Info.bmiHeader.biSize = sizeof(buffer.Info.bmiHeader);
    buffer.Info.bmiHeader.biWidth = buffer.Width;
    buffer.Info.bmiHeader.biHeight = -buffer.Height; // Negative for top-down
    buffer.Info.bmiHeader.biPlanes = 1;
    buffer.Info.bmiHeader.biBitCount = 32;
    buffer.Info.bmiHeader.biCompression = BI_RGB;

    int BitmapMemorySize = (buffer.Width * buffer.Height) * BytesPerPixel;
    buffer.Memory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    buffer.Pitch = width * BytesPerPixel;

    return buffer;
}

void FrameLoader::CleanupFrameBuffer(win32_onscreen_buffer *buffer)
{
    if(buffer->Memory)
    {
        VirtualFree(buffer->Memory, 0, MEM_RELEASE);
        buffer->Memory = nullptr;
    }
}



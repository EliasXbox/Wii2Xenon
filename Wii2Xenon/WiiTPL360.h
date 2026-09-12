#pragma once

#include <xtl.h>

// Read-only Wii TPL parser/decoder used by the 240p bootstrap.
// P0.6a parsed archive metadata. P0.6b adds the first real pixel decoder:
// GX_TF_RGBA8 tiled Wii texture data -> linear Xbox A8R8G8B8 pixels.

struct WiiTPLArchive
{
    const unsigned char* data;
    unsigned int size;
    unsigned int textureCount;
    unsigned int descriptorOffset;
};

struct WiiTPLImageInfo
{
    unsigned short height;
    unsigned short width;
    unsigned int format;
    unsigned int dataOffset;
    unsigned int wrapS;
    unsigned int wrapT;
    unsigned int minFilter;
    unsigned int magFilter;
    float lodBias;
    unsigned char edgeLOD;
    unsigned char minLOD;
    unsigned char maxLOD;
    unsigned char unpacked;
};

bool WiiTPL_OpenMemory(WiiTPLArchive* archive, const void* memory, unsigned int length);
bool WiiTPL_GetImageInfo(const WiiTPLArchive* archive, unsigned int index, WiiTPLImageInfo* info);

// Decode one GX_TF_RGBA8 texture into linear DWORD A8R8G8B8 pixels.
// Returns false for unsupported formats, malformed offsets, or too-small output.
bool WiiTPL_DecodeRGBA8(const WiiTPLArchive* archive,
                         unsigned int index,
                         DWORD* outPixels,
                         unsigned int outPixelCount);

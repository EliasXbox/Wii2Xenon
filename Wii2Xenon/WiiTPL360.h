#pragma once

#include <xtl.h>

// Minimal, read-only Wii TPL parser for the P0.6a bootstrap.
// It intentionally parses offsets instead of rewriting the source buffer,
// so later ports can feed immutable/decompressed TPL blobs safely.

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

#include "WiiTPL360.h"

static unsigned short ReadBE16(const unsigned char* p)
{
    return (unsigned short)(((unsigned short)p[0] << 8) | (unsigned short)p[1]);
}

static unsigned int ReadBE32(const unsigned char* p)
{
    return ((unsigned int)p[0] << 24) |
           ((unsigned int)p[1] << 16) |
           ((unsigned int)p[2] << 8)  |
           (unsigned int)p[3];
}

static float ReadBEFloat(const unsigned char* p)
{
    union
    {
        unsigned int u;
        float f;
    } value;

    value.u = ReadBE32(p);
    return value.f;
}

bool WiiTPL_OpenMemory(WiiTPLArchive* archive, const void* memory, unsigned int length)
{
    if (!archive || !memory || length < 12)
        return false;

    const unsigned char* data = (const unsigned char*)memory;

    // Standard TPL magic/version used by Nintendo/libogc tools.
    if (ReadBE32(data + 0) != 0x0020AF30)
        return false;

    const unsigned int textureCount = ReadBE32(data + 4);
    const unsigned int descriptorOffset = ReadBE32(data + 8);

    if (textureCount == 0)
        return false;

    if (descriptorOffset >= length)
        return false;

    // Each descriptor contains image-header offset + palette-header offset.
    if (textureCount > (length - descriptorOffset) / 8)
        return false;

    archive->data = data;
    archive->size = length;
    archive->textureCount = textureCount;
    archive->descriptorOffset = descriptorOffset;
    return true;
}

bool WiiTPL_GetImageInfo(const WiiTPLArchive* archive, unsigned int index, WiiTPLImageInfo* info)
{
    if (!archive || !archive->data || !info)
        return false;

    if (index >= archive->textureCount)
        return false;

    const unsigned int desc = archive->descriptorOffset + index * 8;
    if (desc + 8 > archive->size)
        return false;

    const unsigned int imageHeaderOffset = ReadBE32(archive->data + desc);

    // Wii TPL image headers are 36 bytes.
    if (imageHeaderOffset > archive->size || archive->size - imageHeaderOffset < 36)
        return false;

    const unsigned char* h = archive->data + imageHeaderOffset;

    info->height = ReadBE16(h + 0);
    info->width = ReadBE16(h + 2);
    info->format = ReadBE32(h + 4);
    info->dataOffset = ReadBE32(h + 8);
    info->wrapS = ReadBE32(h + 12);
    info->wrapT = ReadBE32(h + 16);
    info->minFilter = ReadBE32(h + 20);
    info->magFilter = ReadBE32(h + 24);
    info->lodBias = ReadBEFloat(h + 28);
    info->edgeLOD = h[32];
    info->minLOD = h[33];
    info->maxLOD = h[34];
    info->unpacked = h[35];

    if (info->width == 0 || info->height == 0)
        return false;

    // P0.6a does not decode texture pixels yet, but reject impossible offsets.
    if (info->dataOffset >= archive->size)
        return false;

    return true;
}

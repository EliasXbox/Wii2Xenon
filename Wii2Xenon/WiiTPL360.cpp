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

    if (ReadBE32(data + 0) != 0x0020AF30)
        return false;

    const unsigned int textureCount = ReadBE32(data + 4);
    const unsigned int descriptorOffset = ReadBE32(data + 8);

    if (textureCount == 0)
        return false;

    if (descriptorOffset >= length)
        return false;

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

    if (info->dataOffset >= archive->size)
        return false;

    return true;
}

bool WiiTPL_DecodeRGBA8(const WiiTPLArchive* archive,
                         unsigned int index,
                         DWORD* outPixels,
                         unsigned int outPixelCount)
{
    if (!archive || !outPixels)
        return false;

    WiiTPLImageInfo info;
    ZeroMemory(&info, sizeof(info));

    if (!WiiTPL_GetImageInfo(archive, index, &info))
        return false;

    // GX_TF_RGBA8 = 6. Keep the decoder intentionally strict for P0.6b.
    if (info.format != 6)
        return false;

    const unsigned int pixelCount = (unsigned int)info.width * (unsigned int)info.height;
    if (outPixelCount < pixelCount)
        return false;

    // Wii RGBA8 uses 4x4 tiled blocks, 64 bytes each:
    // first 32 bytes = A,R pairs for 16 pixels
    // next  32 bytes = G,B pairs for the same 16 pixels.
    const unsigned int blocksX = ((unsigned int)info.width + 3) / 4;
    const unsigned int blocksY = ((unsigned int)info.height + 3) / 4;
    const unsigned int requiredBytes = blocksX * blocksY * 64;

    if (info.dataOffset > archive->size || requiredBytes > archive->size - info.dataOffset)
        return false;

    const unsigned char* src = archive->data + info.dataOffset;

    for (unsigned int by = 0; by < blocksY; ++by)
    {
        for (unsigned int bx = 0; bx < blocksX; ++bx)
        {
            const unsigned char* block = src + (by * blocksX + bx) * 64;
            const unsigned char* ar = block;
            const unsigned char* gb = block + 32;

            for (unsigned int py = 0; py < 4; ++py)
            {
                for (unsigned int px = 0; px < 4; ++px)
                {
                    const unsigned int x = bx * 4 + px;
                    const unsigned int y = by * 4 + py;
                    const unsigned int local = py * 4 + px;

                    if (x >= info.width || y >= info.height)
                        continue;

                    const unsigned char a = ar[local * 2 + 0];
                    const unsigned char r = ar[local * 2 + 1];
                    const unsigned char g = gb[local * 2 + 0];
                    const unsigned char b = gb[local * 2 + 1];

                    outPixels[y * info.width + x] = D3DCOLOR_ARGB(a, r, g, b);
                }
            }
        }
    }

    return true;
}

#pragma once

#include <xtl.h>

// Minimal Wii GX-style compatibility surface for early Wii2Xenon tests.
// This is intentionally small and will grow only as real clients require it.

typedef unsigned char  GXU8;
typedef unsigned short GXU16;
typedef unsigned int   GXU32;

enum GXPrimitive
{
    GX_POINTS        = 0xB8,
    GX_LINES         = 0xA8,
    GX_LINESTRIP     = 0xB0,
    GX_TRIANGLES     = 0x90,
    GX_TRIANGLESTRIP = 0x98,
    GX_TRIANGLEFAN   = 0xA0,
    GX_QUADS         = 0x80
};

void GX_Begin(GXU8 primitive, GXU8 vtxfmt, GXU16 vertexCount);
void GX_Position3f32(float x, float y, float z);
void GX_Color4u8(GXU8 r, GXU8 g, GXU8 b, GXU8 a);
void GX_End(void);

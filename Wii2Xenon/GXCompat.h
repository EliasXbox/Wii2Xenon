#pragma once

#include <xtl.h>

// Minimal Wii GX-style compatibility surface for early Wii2Xenon tests.
// This is intentionally small and grows only as real clients require it.

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

enum GXTexMapID
{
    GX_TEXMAP0 = 0
};

enum GXTexWrapMode
{
    GX_CLAMP  = 0,
    GX_REPEAT = 1,
    GX_MIRROR = 2
};

enum GXTexFmt
{
    GX_TF_RGBA8 = 6
};

struct GXTexObj
{
    const void* imageData;
    GXU16 width;
    GXU16 height;
    GXU32 format;
    GXU8 wrapS;
    GXU8 wrapT;
    GXU8 mipmap;
    void* nativeTexture;
};

void GX_Begin(GXU8 primitive, GXU8 vtxfmt, GXU16 vertexCount);
void GX_Position3f32(float x, float y, float z);
void GX_Color4u8(GXU8 r, GXU8 g, GXU8 b, GXU8 a);
void GX_TexCoord2f32(float s, float t);
void GX_End(void);

// M0.4 texture foundation. The current RGBA8 upload path expects a simple
// linear 32-bit ARGB test buffer; native Wii tiled/planar RGBA8 conversion
// will be added later before real Wii assets are treated as compatible.
void GX_InitTexObj(GXTexObj* obj, const void* imageData, GXU16 width, GXU16 height,
    GXU32 format, GXU8 wrapS, GXU8 wrapT, GXU8 mipmap);
void GX_LoadTexObj(GXTexObj* obj, GXU8 mapId);

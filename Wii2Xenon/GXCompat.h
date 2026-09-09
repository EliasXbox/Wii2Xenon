#pragma once
#include <xtl.h>
typedef unsigned char GXU8;
typedef unsigned short GXU16;
typedef unsigned int GXU32;
#define GX_VTXFMT0 0
#define GX_VA_POS 9
#define GX_VA_CLR0 11
#define GX_VA_TEX0 13
#define GX_DIRECT 1
#define GX_POS_XY 0
#define GX_CLR_RGBA 0
#define GX_RGBA8 5
#define GX_TEX_ST 0
#define GX_F32 4
#define GX_DISABLE 0
#define GX_ENABLE 1
#define GX_LEQUAL 3
#define GX_BM_BLEND 1
#define GX_BL_SRCALPHA 4
#define GX_BL_INVSRCALPHA 5
#define GX_LO_CLEAR 0
#define GX_TRUE 1
#define GX_FALSE 0

enum GXPrimitive { GX_POINTS=0xB8, GX_LINES=0xA8, GX_LINESTRIP=0xB0, GX_TRIANGLES=0x90, GX_TRIANGLESTRIP=0x98, GX_TRIANGLEFAN=0xA0, GX_QUADS=0x80 };
enum GXTexMapID { GX_TEXMAP0=0 };
enum GXTexWrapMode { GX_CLAMP=0, GX_REPEAT=1, GX_MIRROR=2 };
enum GXTexFilter { GX_NEAR=0, GX_LINEAR=1 };
enum GXTexFmt { GX_TF_RGBA8=6 };

struct GXTexObj {
 const void* imageData; GXU16 width; GXU16 height; GXU32 format; GXU8 wrapS; GXU8 wrapT; GXU8 mipmap;
 GXU8 minFilter; GXU8 magFilter; float minLOD; float maxLOD; float lodBias; GXU8 biasClamp; GXU8 edgeLOD; GXU8 maxAniso; void* nativeTexture;
};

void GX_Begin(GXU8 primitive,GXU8 vtxfmt,GXU16 vertexCount);
void GX_Position3f32(float x,float y,float z);
void GX_Position2f32(float x,float y);
void GX_Color4u8(GXU8 r,GXU8 g,GXU8 b,GXU8 a);
void GX_TexCoord2f32(float s,float t);
void GX_End(void);
void GX_DrawDone(void);
void GX_InitTexObj(GXTexObj* obj,const void* imageData,GXU16 width,GXU16 height,GXU32 format,GXU8 wrapS,GXU8 wrapT,GXU8 mipmap);
void GX_InitTexObjLOD(GXTexObj* obj,GXU8 minFilter,GXU8 magFilter,float minLOD,float maxLOD,float lodBias,GXU8 biasClamp,GXU8 edgeLOD,GXU8 maxAniso);
void GX_LoadTexObj(GXTexObj* obj,GXU8 mapId);

// P0.2 scene compatibility. Vertex-description calls are accepted as no-ops
// while GX360 uses its current fixed immediate-mode vertex layout.
void GX_SetViewport(float xOrigin,float yOrigin,float width,float height,float nearZ,float farZ);
void GX_InvVtxCache(void);
void GX_ClearVtxDesc(void);
void GX_SetVtxDesc(GXU8 attr,GXU8 type);
void GX_SetVtxAttrFmt(GXU8 vtxfmt,GXU8 attr,GXU8 compCnt,GXU8 compType,GXU8 frac);
void GX_SetZMode(GXU8 enable,GXU8 func,GXU8 updateEnable);
void GX_SetBlendMode(GXU8 type,GXU8 srcFactor,GXU8 dstFactor,GXU8 op);
void GX_SetAlphaUpdate(GXU8 enable);
void GX_SetColorUpdate(GXU8 enable);

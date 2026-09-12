#include <xtl.h>
#include <d3dx9.h>
#include "GX360.h"
#include "WiiXInput.h"
#include "WiiTPL360.h"
#include "gccore.h"
#define WII2XENON 1
extern "C" {
#include "../../240pTestSuite/240psuite/Wii/240pSuite/source/controller.c"
}
#include "GXCompatExtra.cpp"
#include "GXSceneCompat.cpp"
#include "WiiTPL360.cpp"
#include "../../240pTestSuite/240psuite/Wii/240pSuite/source/wii2xenon_image.c"

u8 EndProgram=0;

static const GXU16 P0_TEXTURE_WIDTH=16;
static const GXU16 P0_TEXTURE_HEIGHT=16;
static DWORD g_P0Pixels[P0_TEXTURE_WIDTH*P0_TEXTURE_HEIGHT];
static DWORD g_TPLPixels[16];

static bool LoadPNG16(const char* path) {
 IDirect3DDevice9* device=GX360_GetDevice();
 if(!device) return false;
 IDirect3DTexture9* sourceTexture=NULL;
 HRESULT hr=D3DXCreateTextureFromFileExA(device,path,P0_TEXTURE_WIDTH,P0_TEXTURE_HEIGHT,1,0,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,D3DX_FILTER_NONE,D3DX_FILTER_NONE,0,NULL,NULL,&sourceTexture);
 if(FAILED(hr)||!sourceTexture) return false;
 D3DLOCKED_RECT locked; ZeroMemory(&locked,sizeof(locked));
 hr=sourceTexture->LockRect(0,&locked,NULL,D3DLOCK_READONLY);
 if(FAILED(hr)) { sourceTexture->Release(); return false; }
 for(GXU16 y=0;y<P0_TEXTURE_HEIGHT;++y) {
  const DWORD* s=(const DWORD*)((const BYTE*)locked.pBits+y*locked.Pitch);
  DWORD* d=&g_P0Pixels[y*P0_TEXTURE_WIDTH];
  for(GXU16 x=0;x<P0_TEXTURE_WIDTH;++x) d[x]=s[x];
 }
 sourceTexture->UnlockRect(0); sourceTexture->Release(); return true;
}

static void BuildFallbackTexture(void) {
 for(GXU16 y=0;y<P0_TEXTURE_HEIGHT;++y)
  for(GXU16 x=0;x<P0_TEXTURE_WIDTH;++x) {
   bool a=(((x/2)+(y/2))&1)!=0;
   g_P0Pixels[y*P0_TEXTURE_WIDTH+x]=a?D3DCOLOR_ARGB(255,255,0,255):D3DCOLOR_ARGB(255,25,25,25);
  }
}

static void WriteBE16(unsigned char* p,unsigned short v) { p[0]=(unsigned char)(v>>8); p[1]=(unsigned char)v; }
static void WriteBE32(unsigned char* p,unsigned int v) { p[0]=(unsigned char)(v>>24); p[1]=(unsigned char)(v>>16); p[2]=(unsigned char)(v>>8); p[3]=(unsigned char)v; }

static void PutTPLRGBA8Pixel(unsigned char* block,unsigned int x,unsigned int y,unsigned char a,unsigned char r,unsigned char g,unsigned char b) {
 const unsigned int i=(y*4+x)*2;
 block[i+0]=a; block[i+1]=r;
 block[32+i+0]=g; block[32+i+1]=b;
}

static bool RunP06bTPLDecodeTest(void) {
 // One real 4x4 GX_TF_RGBA8 tiled block inside a valid one-texture TPL.
 // Pattern: red, green, blue, white quadrants. If Wii swizzle/channel decode
 // is correct, the Xbox renderer must reproduce that exact 2x2 quadrant map.
 unsigned char tpl[128]; ZeroMemory(tpl,sizeof(tpl));
 const unsigned int headerOffset=20;
 const unsigned int dataOffset=64;
 WriteBE32(tpl+0,0x0020AF30); WriteBE32(tpl+4,1); WriteBE32(tpl+8,12);
 WriteBE32(tpl+12,headerOffset); WriteBE32(tpl+16,0);
 WriteBE16(tpl+20,4); WriteBE16(tpl+22,4); WriteBE32(tpl+24,GX_TF_RGBA8); WriteBE32(tpl+28,dataOffset);
 WriteBE32(tpl+32,GX_CLAMP); WriteBE32(tpl+36,GX_CLAMP); WriteBE32(tpl+40,GX_NEAR); WriteBE32(tpl+44,GX_NEAR);
 unsigned char* block=tpl+dataOffset;
 for(unsigned int y=0;y<4;++y) for(unsigned int x=0;x<4;++x) {
  if(x<2 && y<2) PutTPLRGBA8Pixel(block,x,y,255,255,0,0);
  else if(x>=2 && y<2) PutTPLRGBA8Pixel(block,x,y,255,0,255,0);
  else if(x<2) PutTPLRGBA8Pixel(block,x,y,255,0,0,255);
  else PutTPLRGBA8Pixel(block,x,y,255,255,255,255);
 }
 WiiTPLArchive archive; ZeroMemory(&archive,sizeof(archive));
 if(!WiiTPL_OpenMemory(&archive,tpl,sizeof(tpl))) return false;
 if(!WiiTPL_DecodeRGBA8(&archive,0,g_TPLPixels,16)) return false;
 return g_TPLPixels[0]==D3DCOLOR_ARGB(255,255,0,0) &&
        g_TPLPixels[3]==D3DCOLOR_ARGB(255,0,255,0) &&
        g_TPLPixels[12]==D3DCOLOR_ARGB(255,0,0,255) &&
        g_TPLPixels[15]==D3DCOLOR_ARGB(255,255,255,255);
}

VOID __cdecl main() {
 OutputDebugStringA("============================================\n");
 OutputDebugStringA(" 240p Test Suite - Wii2Xenon P0.6b\n");
 OutputDebugStringA(" First GX_TF_RGBA8 TPL pixel decode\n");
 OutputDebugStringA("============================================\n");
 if(!GX360_Init()) { OutputDebugStringA("[240p/Wii2Xenon] FATAL: GX360 init failed.\n"); for(;;) Sleep(1000); }
 ControllerInit();
 const bool tplDecodeOk=RunP06bTPLDecodeTest();
 OutputDebugStringA(tplDecodeOk?"[Wii2Xenon/TPL] P0.6b PASS: GX_TF_RGBA8 tiled pixels decoded.\n":"[Wii2Xenon/TPL] P0.6b FAIL: RGBA8 decode mismatch.\n");
 if(!LoadPNG16("game:\\assets\\Convergence-01-grid.png")) BuildFallbackTexture();

 struct image_st tplImage; ZeroMemory(&tplImage,sizeof(tplImage));
 GX_InitTexObj(&tplImage.tex,g_TPLPixels,4,4,GX_TF_RGBA8,GX_CLAMP,GX_CLAMP,GX_FALSE);
 GX_InitTexObjLOD(&tplImage.tex,GX_NEAR,GX_NEAR,0,0,0,0,0,0);
 tplImage.x=32; tplImage.y=56; tplImage.w=112; tplImage.h=112; tplImage.tw=4; tplImage.th=4;
 tplImage.u1=0; tplImage.v1=0; tplImage.u2=1; tplImage.v2=1; tplImage.r=255; tplImage.g=255; tplImage.b=255; tplImage.alpha=255; tplImage.IgnoreOffsetY=1;

 struct image_st regression; ZeroMemory(&regression,sizeof(regression));
 GX_InitTexObj(&regression.tex,g_P0Pixels,16,16,GX_TF_RGBA8,GX_CLAMP,GX_CLAMP,GX_FALSE);
 GX_InitTexObjLOD(&regression.tex,GX_NEAR,GX_NEAR,0,0,0,0,0,0);
 regression.x=176; regression.y=80; regression.w=96; regression.h=96; regression.tw=16; regression.th=16;
 regression.u1=0; regression.v1=0; regression.u2=1; regression.v2=1; regression.r=255; regression.g=255; regression.b=255; regression.alpha=255; regression.IgnoreOffsetY=1;

 for(;;) {
  ControllerScan(); const u32 pressed=Controller_ButtonsDown(0);
  if(pressed&PAD_BUTTON_A) { regression.r=255; regression.g=255; regression.b=255; }
  else if(pressed&PAD_BUTTON_B) { regression.r=255; regression.g=90; regression.b=90; }
  else if(pressed&PAD_BUTTON_X) { regression.r=90; regression.g=150; regression.b=255; }
  else if(pressed&PAD_BUTTON_Y) { regression.r=255; regression.g=220; regression.b=90; }
  GX360_Clear(tplDecodeOk?D3DCOLOR_XRGB(8,42,20):D3DCOLOR_XRGB(72,8,18));
  StartScene(); DrawImage(&tplImage); DrawImage(&regression); EndScene();
 }
}

#include <xtl.h>
#include <d3dx9.h>
#include "GX360.h"
#include "WiiXInput.h"
#include "gccore.h"
#define WII2XENON 1
extern "C" {
#include "../../240pTestSuite/240psuite/Wii/240pSuite/source/controller.c"
}
#include "GXCompatExtra.cpp"
#include "GXSceneCompat.cpp"
#include "../../240pTestSuite/240psuite/Wii/240pSuite/source/wii2xenon_image.c"

u8 EndProgram=0;

// P0.5: first real visual asset from the 240p Test Suite repository.
// Convergence-01-grid.png is 16x16, but we draw it enlarged so the result is
// easy to verify on both Xenia and a real Xbox 360.
static const GXU16 P0_TEXTURE_WIDTH=16;
static const GXU16 P0_TEXTURE_HEIGHT=16;
static DWORD g_P0Pixels[P0_TEXTURE_WIDTH*P0_TEXTURE_HEIGHT];

static bool LoadPNG16(const char* path) {
 IDirect3DDevice9* device=GX360_GetDevice();
 if(!device) return false;

 IDirect3DTexture9* sourceTexture=NULL;
 HRESULT hr=D3DXCreateTextureFromFileExA(
  device,path,
  P0_TEXTURE_WIDTH,P0_TEXTURE_HEIGHT,
  1,0,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,
  D3DX_FILTER_NONE,D3DX_FILTER_NONE,0,
  NULL,NULL,&sourceTexture);

 if(FAILED(hr)||!sourceTexture) return false;

 D3DLOCKED_RECT locked;
 ZeroMemory(&locked,sizeof(locked));
 hr=sourceTexture->LockRect(0,&locked,NULL,D3DLOCK_READONLY);
 if(FAILED(hr)) { sourceTexture->Release(); return false; }

 for(GXU16 y=0;y<P0_TEXTURE_HEIGHT;++y) {
  const DWORD* s=(const DWORD*)((const BYTE*)locked.pBits+y*locked.Pitch);
  DWORD* d=&g_P0Pixels[y*P0_TEXTURE_WIDTH];
  for(GXU16 x=0;x<P0_TEXTURE_WIDTH;++x) d[x]=s[x];
 }

 sourceTexture->UnlockRect(0);
 sourceTexture->Release();
 return true;
}

static void BuildFallbackTexture(void) {
 for(GXU16 y=0;y<P0_TEXTURE_HEIGHT;++y)
  for(GXU16 x=0;x<P0_TEXTURE_WIDTH;++x) {
   bool a=(((x/2)+(y/2))&1)!=0;
   g_P0Pixels[y*P0_TEXTURE_WIDTH+x]=a
    ? D3DCOLOR_ARGB(255,255,0,255)
    : D3DCOLOR_ARGB(255,25,25,25);
  }
}

VOID __cdecl main() {
 OutputDebugStringA("============================================\n");
 OutputDebugStringA(" 240p Test Suite - Wii2Xenon P0.5\n");
 OutputDebugStringA(" First real 240p Test Suite asset\n");
 OutputDebugStringA("============================================\n");

 if(!GX360_Init()) {
  OutputDebugStringA("[240p/Wii2Xenon] FATAL: GX360 init failed.\n");
  for(;;) Sleep(1000);
 }

 ControllerInit();

 const char* assetPath="game:\\assets\\Convergence-01-grid.png";
 if(!LoadPNG16(assetPath)) {
  OutputDebugStringA("[240p/Wii2Xenon] P0.5 asset missing: game:\\assets\\Convergence-01-grid.png\n");
  OutputDebugStringA("[240p/Wii2Xenon] Using magenta fallback checker.\n");
  BuildFallbackTexture();
 } else {
  OutputDebugStringA("[240p/Wii2Xenon] Loaded real 240p asset: Convergence-01-grid.png\n");
 }

 struct image_st image;
 ZeroMemory(&image,sizeof(image));

 GX_InitTexObj(&image.tex,g_P0Pixels,
  P0_TEXTURE_WIDTH,P0_TEXTURE_HEIGHT,
  GX_TF_RGBA8,GX_CLAMP,GX_CLAMP,GX_FALSE);

 // NEAR is intentional: this is a tiny pixel-art calibration asset and P0.5
 // should preserve its exact hard-edged source pixels while magnified.
 GX_InitTexObjLOD(&image.tex,GX_NEAR,GX_NEAR,0,0,0,0,0,0);

 image.x=96;
 image.y=56;
 image.w=128;
 image.h=128;
 image.tw=(float)P0_TEXTURE_WIDTH;
 image.th=(float)P0_TEXTURE_HEIGHT;
 image.u1=0;
 image.v1=0;
 image.u2=1;
 image.v2=1;
 image.r=255;
 image.g=255;
 image.b=255;
 image.alpha=255;
 image.scale=0;
 image.IgnoreOffsetY=1;

 OutputDebugStringA("[240p/Wii2Xenon] P0.5 ready: real 240p PNG -> GXTexObj -> DrawImage -> GX360.\n");
 OutputDebugStringA("[240p/Wii2Xenon] A/B/X/Y still tint the asset as an input/render regression test.\n");

 for(;;) {
  ControllerScan();
  const u32 pressed=Controller_ButtonsDown(0);

  if(pressed&PAD_BUTTON_A) { image.r=255; image.g=255; image.b=255; }
  else if(pressed&PAD_BUTTON_B) { image.r=255; image.g=90; image.b=90; }
  else if(pressed&PAD_BUTTON_X) { image.r=90; image.g=150; image.b=255; }
  else if(pressed&PAD_BUTTON_Y) { image.r=255; image.g=220; image.b=90; }

  GX360_Clear(D3DCOLOR_XRGB(18,22,32));
  StartScene();
  DrawImage(&image);
  EndScene();
 }
}

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
static const GXU16 P0_TEXTURE_SIZE=128;
static DWORD g_P0Pixels[P0_TEXTURE_SIZE*P0_TEXTURE_SIZE];

static bool LoadPNG128(const char* path) {
 IDirect3DDevice9* device=GX360_GetDevice(); if(!device) return false;
 IDirect3DTexture9* sourceTexture=NULL;
 HRESULT hr=D3DXCreateTextureFromFileExA(device,path,P0_TEXTURE_SIZE,P0_TEXTURE_SIZE,1,0,D3DFMT_A8R8G8B8,D3DPOOL_DEFAULT,D3DX_FILTER_NONE,D3DX_FILTER_NONE,0,NULL,NULL,&sourceTexture);
 if(FAILED(hr)||!sourceTexture) return false;
 D3DLOCKED_RECT locked; ZeroMemory(&locked,sizeof(locked)); hr=sourceTexture->LockRect(0,&locked,NULL,D3DLOCK_READONLY);
 if(FAILED(hr)){sourceTexture->Release();return false;}
 for(GXU16 y=0;y<P0_TEXTURE_SIZE;++y){const DWORD* s=(const DWORD*)((const BYTE*)locked.pBits+y*locked.Pitch); DWORD* d=&g_P0Pixels[y*P0_TEXTURE_SIZE]; for(GXU16 x=0;x<P0_TEXTURE_SIZE;++x)d[x]=s[x];}
 sourceTexture->UnlockRect(0); sourceTexture->Release(); return true;
}

static void BuildFallbackTexture(void) {
 for(GXU16 y=0;y<P0_TEXTURE_SIZE;++y) for(GXU16 x=0;x<P0_TEXTURE_SIZE;++x) {
  bool a=(((x/8)+(y/8))&1)!=0; g_P0Pixels[y*P0_TEXTURE_SIZE+x]=a?D3DCOLOR_ARGB(255,230,230,230):D3DCOLOR_ARGB(255,40,55,75);
 }
}

VOID __cdecl main() {
 OutputDebugStringA("============================================\n 240p Test Suite - Wii2Xenon P0.2\n StartScene / DrawImage / EndScene bootstrap\n============================================\n");
 if(!GX360_Init()){OutputDebugStringA("[240p/Wii2Xenon] FATAL: GX360 init failed.\n");for(;;)Sleep(1000);}
 ControllerInit();
 if(!LoadPNG128("game:\\placeholders\\placeholder4.png")){OutputDebugStringA("[240p/Wii2Xenon] PNG missing; fallback.\n");BuildFallbackTexture();}
 struct image_st image; ZeroMemory(&image,sizeof(image));
 GX_InitTexObj(&image.tex,g_P0Pixels,P0_TEXTURE_SIZE,P0_TEXTURE_SIZE,GX_TF_RGBA8,GX_CLAMP,GX_CLAMP,GX_FALSE);
 GX_InitTexObjLOD(&image.tex,GX_LINEAR,GX_LINEAR,0,0,0,0,0,0);
 image.x=96; image.y=56; image.w=128; image.h=128; image.tw=128; image.th=128; image.u1=0; image.v1=0; image.u2=1; image.v2=1; image.r=255; image.g=255; image.b=255; image.alpha=255;
 OutputDebugStringA("[240p/Wii2Xenon] P0.2 ready: StartScene -> DrawImage -> EndScene.\n");
 for(;;) {
  ControllerScan(); const u32 pressed=Controller_ButtonsDown(0);
  if(pressed&PAD_BUTTON_A){image.r=255;image.g=255;image.b=255;}
  else if(pressed&PAD_BUTTON_B){image.r=255;image.g=90;image.b=90;}
  else if(pressed&PAD_BUTTON_X){image.r=90;image.g=150;image.b=255;}
  else if(pressed&PAD_BUTTON_Y){image.r=255;image.g=220;image.b=90;}
  GX360_Clear(D3DCOLOR_XRGB(18,22,32));
  StartScene();
  DrawImage(&image);
  EndScene();
 }
}

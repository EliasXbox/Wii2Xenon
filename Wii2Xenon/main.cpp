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

static void WriteBE16(unsigned char* p,unsigned short value) {
 p[0]=(unsigned char)((value>>8)&0xFF);
 p[1]=(unsigned char)(value&0xFF);
}

static void WriteBE32(unsigned char* p,unsigned int value) {
 p[0]=(unsigned char)((value>>24)&0xFF);
 p[1]=(unsigned char)((value>>16)&0xFF);
 p[2]=(unsigned char)((value>>8)&0xFF);
 p[3]=(unsigned char)(value&0xFF);
}

static bool RunP06aTPLParserTest(void) {
 unsigned char tpl[64];
 ZeroMemory(tpl,sizeof(tpl));

 WriteBE32(tpl+0,0x0020AF30);
 WriteBE32(tpl+4,1);
 WriteBE32(tpl+8,12);
 WriteBE32(tpl+12,20);
 WriteBE32(tpl+16,0);

 WriteBE16(tpl+20,16);
 WriteBE16(tpl+22,16);
 WriteBE32(tpl+24,GX_TF_RGBA8);
 WriteBE32(tpl+28,56);
 WriteBE32(tpl+32,GX_CLAMP);
 WriteBE32(tpl+36,GX_CLAMP);
 WriteBE32(tpl+40,GX_NEAR);
 WriteBE32(tpl+44,GX_NEAR);
 WriteBE32(tpl+48,0);
 tpl[52]=0;
 tpl[53]=0;
 tpl[54]=0;
 tpl[55]=0;
 tpl[56]=0xAA;

 WiiTPLArchive archive;
 ZeroMemory(&archive,sizeof(archive));
 WiiTPLImageInfo info;
 ZeroMemory(&info,sizeof(info));

 if(!WiiTPL_OpenMemory(&archive,tpl,sizeof(tpl)))
  return false;
 if(archive.textureCount!=1)
  return false;
 if(!WiiTPL_GetImageInfo(&archive,0,&info))
  return false;

 return info.width==16 &&
        info.height==16 &&
        info.format==GX_TF_RGBA8 &&
        info.dataOffset==56 &&
        info.wrapS==GX_CLAMP &&
        info.wrapT==GX_CLAMP;
}

VOID __cdecl main() {
 OutputDebugStringA("============================================\n");
 OutputDebugStringA(" 240p Test Suite - Wii2Xenon P0.6a\n");
 OutputDebugStringA(" TPL parser metadata bootstrap\n");
 OutputDebugStringA("============================================\n");

 if(!GX360_Init()) {
  OutputDebugStringA("[240p/Wii2Xenon] FATAL: GX360 init failed.\n");
  for(;;) Sleep(1000);
 }

 ControllerInit();

 const bool tplParserOk=RunP06aTPLParserTest();
 if(tplParserOk)
  OutputDebugStringA("[Wii2Xenon/TPL] P0.6a PASS: TPL header/descriptor/image metadata parsed correctly.\n");
 else
  OutputDebugStringA("[Wii2Xenon/TPL] P0.6a FAIL: TPL metadata parser rejected the bootstrap blob.\n");

 const char* assetPath="game:\\assets\\Convergence-01-grid.png";
 if(!LoadPNG16(assetPath)) {
  OutputDebugStringA("[240p/Wii2Xenon] P0.5 regression asset missing.\n");
  BuildFallbackTexture();
 } else {
  OutputDebugStringA("[240p/Wii2Xenon] P0.5 regression asset loaded.\n");
 }

 struct image_st image;
 ZeroMemory(&image,sizeof(image));
 GX_InitTexObj(&image.tex,g_P0Pixels,P0_TEXTURE_WIDTH,P0_TEXTURE_HEIGHT,GX_TF_RGBA8,GX_CLAMP,GX_CLAMP,GX_FALSE);
 GX_InitTexObjLOD(&image.tex,GX_NEAR,GX_NEAR,0,0,0,0,0,0);

 image.x=96; image.y=56; image.w=128; image.h=128;
 image.tw=(float)P0_TEXTURE_WIDTH; image.th=(float)P0_TEXTURE_HEIGHT;
 image.u1=0; image.v1=0; image.u2=1; image.v2=1;
 image.r=255; image.g=255; image.b=255; image.alpha=255;
 image.scale=0; image.IgnoreOffsetY=1;

 OutputDebugStringA("[Wii2Xenon/TPL] Green background = parser PASS; dark red = parser FAIL.\n");

 for(;;) {
  ControllerScan();
  const u32 pressed=Controller_ButtonsDown(0);

  if(pressed&PAD_BUTTON_A) { image.r=255; image.g=255; image.b=255; }
  else if(pressed&PAD_BUTTON_B) { image.r=255; image.g=90; image.b=90; }
  else if(pressed&PAD_BUTTON_X) { image.r=90; image.g=150; image.b=255; }
  else if(pressed&PAD_BUTTON_Y) { image.r=255; image.g=220; image.b=90; }

  GX360_Clear(tplParserOk ? D3DCOLOR_XRGB(8,42,20) : D3DCOLOR_XRGB(72,8,18));
  StartScene();
  DrawImage(&image);
  EndScene();
 }
}

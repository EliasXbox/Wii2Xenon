#include <xtl.h>
#include <d3dx9.h>
#include "GX360.h"
#include "WiiXInput.h"
#include "gccore.h"

#define WII2XENON 1

// P0.1 still consumes the original 240p controller implementation directly
// from the sibling checkout. The image API comes from the 240p branch too,
// with a branch-only DrawImage adapter that targets GX360.
extern "C"
{
#include "../../240pTestSuite/240psuite/Wii/240pSuite/source/controller.c"
}

#include "../../240pTestSuite/240psuite/Wii/240pSuite/source/wii2xenon_image.c"
#include "GXCompatExtra.cpp"

u8 EndProgram = 0;

static const GXU16 P0_TEXTURE_SIZE = 128;
static DWORD g_P0Pixels[P0_TEXTURE_SIZE * P0_TEXTURE_SIZE];

static bool LoadPNG128(const char* path)
{
    IDirect3DDevice9* device = GX360_GetDevice();
    if (device == NULL)
        return false;

    IDirect3DTexture9* sourceTexture = NULL;
    HRESULT hr = D3DXCreateTextureFromFileExA(
        device,
        path,
        P0_TEXTURE_SIZE,
        P0_TEXTURE_SIZE,
        1,
        0,
        D3DFMT_A8R8G8B8,
        D3DPOOL_DEFAULT,
        D3DX_FILTER_NONE,
        D3DX_FILTER_NONE,
        0,
        NULL,
        NULL,
        &sourceTexture);

    if (FAILED(hr) || sourceTexture == NULL)
        return false;

    D3DLOCKED_RECT locked;
    ZeroMemory(&locked, sizeof(locked));
    hr = sourceTexture->LockRect(0, &locked, NULL, D3DLOCK_READONLY);
    if (FAILED(hr))
    {
        sourceTexture->Release();
        return false;
    }

    for (GXU16 y = 0; y < P0_TEXTURE_SIZE; ++y)
    {
        const DWORD* source = (const DWORD*)((const BYTE*)locked.pBits + y * locked.Pitch);
        DWORD* destination = &g_P0Pixels[y * P0_TEXTURE_SIZE];
        for (GXU16 x = 0; x < P0_TEXTURE_SIZE; ++x)
            destination[x] = source[x];
    }

    sourceTexture->UnlockRect(0);
    sourceTexture->Release();
    return true;
}

static void BuildFallbackTexture(void)
{
    for (GXU16 y = 0; y < P0_TEXTURE_SIZE; ++y)
    {
        for (GXU16 x = 0; x < P0_TEXTURE_SIZE; ++x)
        {
            const bool alternate = (((x / 8) + (y / 8)) & 1) != 0;
            g_P0Pixels[y * P0_TEXTURE_SIZE + x] = alternate
                ? D3DCOLOR_ARGB(255, 230, 230, 230)
                : D3DCOLOR_ARGB(255, 40, 55, 75);
        }
    }
}

VOID __cdecl main()
{
    OutputDebugStringA("============================================\n");
    OutputDebugStringA(" 240p Test Suite - Wii2Xenon P0.1\n");
    OutputDebugStringA(" First ImagePtr / DrawImage visual bootstrap\n");
    OutputDebugStringA("============================================\n");

    if (!GX360_Init())
    {
        OutputDebugStringA("[240p/Wii2Xenon] FATAL: GX360 initialization failed!\n");
        for (;;)
            Sleep(1000);
    }

    ControllerInit();

    if (!LoadPNG128("game:\\placeholders\\placeholder4.png"))
    {
        OutputDebugStringA("[240p/Wii2Xenon] P0.1 PNG missing; using fallback texture.\n");
        BuildFallbackTexture();
    }

    struct image_st image;
    ZeroMemory(&image, sizeof(image));

    GX_InitTexObj(&image.tex, g_P0Pixels,
        P0_TEXTURE_SIZE, P0_TEXTURE_SIZE,
        GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, GX_FALSE);
    GX_InitTexObjLOD(&image.tex,
        GX_LINEAR, GX_LINEAR,
        0.0f, 0.0f, 0.0f,
        0, 0, 0);

    // Use the real 240p image structure and its 320x240 coordinate style.
    image.x = 96.0f;
    image.y = 56.0f;
    image.w = 128.0f;
    image.h = 128.0f;
    image.tw = 128.0f;
    image.th = 128.0f;
    image.u1 = 0.0f;
    image.v1 = 0.0f;
    image.u2 = 1.0f;
    image.v2 = 1.0f;
    image.r = 255;
    image.g = 255;
    image.b = 255;
    image.alpha = 255;

    OutputDebugStringA("[240p/Wii2Xenon] P0.1 ready: 240p ImagePtr -> DrawImage -> GX360.\n");
    OutputDebugStringA("[240p/Wii2Xenon] A/B/X/Y changes ImagePtr tint through original 240p controller code.\n");

    for (;;)
    {
        ControllerScan();
        const u32 pressed = Controller_ButtonsDown(0);

        if (pressed & PAD_BUTTON_A)
        {
            image.r = 255; image.g = 255; image.b = 255;
        }
        else if (pressed & PAD_BUTTON_B)
        {
            image.r = 255; image.g = 90; image.b = 90;
        }
        else if (pressed & PAD_BUTTON_X)
        {
            image.r = 90; image.g = 150; image.b = 255;
        }
        else if (pressed & PAD_BUTTON_Y)
        {
            image.r = 255; image.g = 220; image.b = 90;
        }

        GX360_Clear(D3DCOLOR_XRGB(18, 22, 32));
        DrawImage(&image);
        GX360_Present();
    }
}

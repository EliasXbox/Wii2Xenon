#include <xtl.h>
#include <d3dx9.h>
#include "GX360.h"
#include "GXCompat.h"
#include "WiiXInput.h"

// ============================================================
// Wii2Xenon - M0.4.2
// External PNG -> RGBA/ARGB pixels -> GXTexObj -> GX360
// ============================================================

static const GXU16 PNG_TEXTURE_SIZE = 128;
static DWORD g_PngPixels[PNG_TEXTURE_SIZE * PNG_TEXTURE_SIZE];

static bool LoadPNG128(const char* path)
{
    IDirect3DDevice9* device = GX360_GetDevice();
    if (device == NULL)
        return false;

    IDirect3DTexture9* sourceTexture = NULL;
    HRESULT hr = D3DXCreateTextureFromFileExA(
        device,
        path,
        PNG_TEXTURE_SIZE,
        PNG_TEXTURE_SIZE,
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
    {
        OutputDebugStringA("[Wii2Xenon/M0.4.2] PNG load failed.\n");
        return false;
    }

    D3DLOCKED_RECT locked;
    ZeroMemory(&locked, sizeof(locked));
    hr = sourceTexture->LockRect(0, &locked, NULL, D3DLOCK_READONLY);
    if (FAILED(hr))
    {
        sourceTexture->Release();
        OutputDebugStringA("[Wii2Xenon/M0.4.2] PNG texture LockRect failed.\n");
        return false;
    }

    for (GXU16 y = 0; y < PNG_TEXTURE_SIZE; ++y)
    {
        const DWORD* source = (const DWORD*)((const BYTE*)locked.pBits + y * locked.Pitch);
        DWORD* destination = &g_PngPixels[y * PNG_TEXTURE_SIZE];
        for (GXU16 x = 0; x < PNG_TEXTURE_SIZE; ++x)
            destination[x] = source[x];
    }

    sourceTexture->UnlockRect(0);
    sourceTexture->Release();

    OutputDebugStringA("[Wii2Xenon/M0.4.2] PNG decoded to 128x128 ARGB pixels.\n");
    return true;
}

static void BuildFallbackTexture(void)
{
    for (GXU16 y = 0; y < PNG_TEXTURE_SIZE; ++y)
    {
        for (GXU16 x = 0; x < PNG_TEXTURE_SIZE; ++x)
        {
            const bool alternate = (((x / 8) + (y / 8)) & 1) != 0;
            g_PngPixels[y * PNG_TEXTURE_SIZE + x] = alternate
                ? D3DCOLOR_ARGB(255, 255, 70, 220)
                : D3DCOLOR_ARGB(255, 35, 35, 45);
        }
    }
}

static void DrawTexture(GXTexObj* texture)
{
    GX_LoadTexObj(texture, GX_TEXMAP0);
    GX_Begin(GX_QUADS, 0, 4);

    GX_Position3f32(-0.55f, -0.73f, 0.0f);
    GX_Color4u8(255, 255, 255, 255);
    GX_TexCoord2f32(0.0f, 1.0f);

    GX_Position3f32(0.55f, -0.73f, 0.0f);
    GX_Color4u8(255, 255, 255, 255);
    GX_TexCoord2f32(1.0f, 1.0f);

    GX_Position3f32(0.55f, 0.73f, 0.0f);
    GX_Color4u8(255, 255, 255, 255);
    GX_TexCoord2f32(1.0f, 0.0f);

    GX_Position3f32(-0.55f, 0.73f, 0.0f);
    GX_Color4u8(255, 255, 255, 255);
    GX_TexCoord2f32(0.0f, 0.0f);

    GX_End();
}

VOID __cdecl main()
{
    OutputDebugStringA("============================================\n");
    OutputDebugStringA(" Wii2Xenon Runtime - M0.4.2\n");
    OutputDebugStringA(" External PNG Texture Test\n");
    OutputDebugStringA("============================================\n");

    if (!GX360_Init())
    {
        OutputDebugStringA("[Wii2Xenon] FATAL: GX360 initialization failed!\n");
        for (;;)
            Sleep(1000);
    }

    PAD_Init();
    bool rumbleEnabled = false;

    // First try the path requested for the M0.4.2 placeholder. If the PNG
    // isn't present beside the XEX/runtime working directory, the fallback
    // texture makes that failure immediately visible instead of a black quad.
    if (!LoadPNG128("placeholders\\placeholder4.png"))
    {
        OutputDebugStringA("[Wii2Xenon/M0.4.2] Using fallback texture. Copy placeholders/placeholder4.png beside the XEX tree.\n");
        BuildFallbackTexture();
    }

    GXTexObj pngTexture;
    ZeroMemory(&pngTexture, sizeof(pngTexture));
    GX_InitTexObj(&pngTexture, g_PngPixels,
        PNG_TEXTURE_SIZE, PNG_TEXTURE_SIZE,
        GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, 0);
    GX_InitTexObjLOD(&pngTexture,
        GX_LINEAR, GX_LINEAR,
        0.0f, 0.0f, 0.0f,
        0, 0, 0);

    OutputDebugStringA("[Wii2Xenon/M0.4.2] Rendering external PNG through GX360.\n");
    OutputDebugStringA("[Wii2Xenon] Hold A for rumble regression test.\n");

    for (;;)
    {
        PAD_ScanPads();
        const u16 held = PAD_ButtonsHeld(PAD_CHAN0);

        const bool wantsRumble = (held & PAD_BUTTON_A) != 0;
        if (wantsRumble != rumbleEnabled)
        {
            PAD_ControlMotor(PAD_CHAN0,
                wantsRumble ? PAD_MOTOR_RUMBLE : PAD_MOTOR_STOP);
            rumbleEnabled = wantsRumble;
        }

        GX360_Clear(D3DCOLOR_XRGB(24, 28, 40));
        DrawTexture(&pngTexture);
        GX360_Present();
    }
}

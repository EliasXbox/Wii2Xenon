#include <xtl.h>
#include <d3dx9.h>
#include "GX360.h"
#include "GXCompat.h"
#include "WiiXInput.h"

// ============================================================
// Wii2Xenon - M0.4.3
// Four external PNG textures + runtime switching through PAD.
// A -> placeholder4, B -> placeholder2,
// X -> placeholder3, Y -> placeholder1.
// ============================================================

static const GXU16 PNG_TEXTURE_SIZE = 128;
static const int PNG_TEXTURE_COUNT = 4;
static DWORD g_PngPixels[PNG_TEXTURE_COUNT][PNG_TEXTURE_SIZE * PNG_TEXTURE_SIZE];
static GXTexObj g_PngTextures[PNG_TEXTURE_COUNT];

static const char* g_PngPaths[PNG_TEXTURE_COUNT] =
{
    "game:\\placeholders\\placeholder4.png", // A
    "game:\\placeholders\\placeholder2.png", // B
    "game:\\placeholders\\placeholder3.png", // X
    "game:\\placeholders\\placeholder1.png"  // Y
};

static void DebugPath(const char* prefix, const char* path)
{
    OutputDebugStringA(prefix);
    OutputDebugStringA(path);
    OutputDebugStringA("\n");
}

static bool LoadPNG128(const char* path, DWORD* destinationPixels)
{
    IDirect3DDevice9* device = GX360_GetDevice();
    if (device == NULL)
        return false;

    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == INVALID_HANDLE_VALUE)
    {
        DebugPath("[M0.4.3] MISS: ", path);
        return false;
    }
    CloseHandle(file);

    DebugPath("[M0.4.3] FOUND: ", path);

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
        DebugPath("[M0.4.3] D3DX decode FAILED: ", path);
        return false;
    }

    D3DLOCKED_RECT locked;
    ZeroMemory(&locked, sizeof(locked));
    hr = sourceTexture->LockRect(0, &locked, NULL, D3DLOCK_READONLY);
    if (FAILED(hr))
    {
        sourceTexture->Release();
        DebugPath("[M0.4.3] LockRect FAILED: ", path);
        return false;
    }

    for (GXU16 y = 0; y < PNG_TEXTURE_SIZE; ++y)
    {
        const DWORD* source = (const DWORD*)((const BYTE*)locked.pBits + y * locked.Pitch);
        DWORD* destination = &destinationPixels[y * PNG_TEXTURE_SIZE];
        for (GXU16 x = 0; x < PNG_TEXTURE_SIZE; ++x)
            destination[x] = source[x];
    }

    sourceTexture->UnlockRect(0);
    sourceTexture->Release();
    DebugPath("[M0.4.3] DECODED: ", path);
    return true;
}

static void BuildFallbackTexture(DWORD* pixels, int index)
{
    for (GXU16 y = 0; y < PNG_TEXTURE_SIZE; ++y)
    {
        for (GXU16 x = 0; x < PNG_TEXTURE_SIZE; ++x)
        {
            const bool alternate = (((x / 8) + (y / 8) + index) & 1) != 0;
            pixels[y * PNG_TEXTURE_SIZE + x] = alternate
                ? D3DCOLOR_ARGB(255, 255, 70, 220)
                : D3DCOLOR_ARGB(255, 35, 35, 45);
        }
    }
}

static void InitTexture(int index)
{
    if (!LoadPNG128(g_PngPaths[index], g_PngPixels[index]))
        BuildFallbackTexture(g_PngPixels[index], index);

    ZeroMemory(&g_PngTextures[index], sizeof(GXTexObj));
    GX_InitTexObj(&g_PngTextures[index], g_PngPixels[index],
        PNG_TEXTURE_SIZE, PNG_TEXTURE_SIZE,
        GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, 0);
    GX_InitTexObjLOD(&g_PngTextures[index],
        GX_LINEAR, GX_LINEAR,
        0.0f, 0.0f, 0.0f,
        0, 0, 0);
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
    OutputDebugStringA(" Wii2Xenon Runtime - M0.4.3\n");
    OutputDebugStringA(" Four PNG Runtime Texture Switching\n");
    OutputDebugStringA(" A=4 B=2 X=3 Y=1\n");
    OutputDebugStringA("============================================\n");

    if (!GX360_Init())
    {
        OutputDebugStringA("[Wii2Xenon] FATAL: GX360 initialization failed!\n");
        for (;;)
            Sleep(1000);
    }

    PAD_Init();

    for (int i = 0; i < PNG_TEXTURE_COUNT; ++i)
        InitTexture(i);

    int activeTexture = 0;
    bool rumbleEnabled = false;

    OutputDebugStringA("[M0.4.3] Ready. A/B/X/Y switches external PNG texture.\n");

    for (;;)
    {
        PAD_ScanPads();
        const u16 held = PAD_ButtonsHeld(PAD_CHAN0);
        const u16 down = PAD_ButtonsDown(PAD_CHAN0);

        if (down & PAD_BUTTON_A)
            activeTexture = 0;
        else if (down & PAD_BUTTON_B)
            activeTexture = 1;
        else if (down & PAD_BUTTON_X)
            activeTexture = 2;
        else if (down & PAD_BUTTON_Y)
            activeTexture = 3;

        const bool wantsRumble = (held & PAD_BUTTON_A) != 0;
        if (wantsRumble != rumbleEnabled)
        {
            PAD_ControlMotor(PAD_CHAN0,
                wantsRumble ? PAD_MOTOR_RUMBLE : PAD_MOTOR_STOP);
            rumbleEnabled = wantsRumble;
        }

        GX360_Clear(D3DCOLOR_XRGB(24, 28, 40));
        DrawTexture(&g_PngTextures[activeTexture]);
        GX360_Present();
    }
}

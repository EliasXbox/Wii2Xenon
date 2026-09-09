#include <xtl.h>
#include <d3dx9.h>
#include "GX360.h"
#include "GXCompat.h"
#include "WiiXInput.h"

// ============================================================
// Wii2Xenon - M0.4.2a
// Diagnose Xbox filesystem path first, then PNG decoding.
// ============================================================

static const GXU16 PNG_TEXTURE_SIZE = 128;
static DWORD g_PngPixels[PNG_TEXTURE_SIZE * PNG_TEXTURE_SIZE];

static bool FileExists(const char* path)
{
    HANDLE file = CreateFileA(path, GENERIC_READ, FILE_SHARE_READ, NULL,
        OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

    if (file == INVALID_HANDLE_VALUE)
        return false;

    CloseHandle(file);
    return true;
}

static void DebugPathResult(const char* path, bool exists)
{
    OutputDebugStringA(exists ? "[M0.4.2a] FOUND: " : "[M0.4.2a] MISS : ");
    OutputDebugStringA(path);
    OutputDebugStringA("\n");
}

static const char* FindPlaceholderPath(void)
{
    static const char* paths[] =
    {
        "game:\\placeholders\\placeholder4.png",
        "D:\\placeholders\\placeholder4.png",
        "placeholders\\placeholder4.png"
    };

    for (int i = 0; i < 3; ++i)
    {
        const bool exists = FileExists(paths[i]);
        DebugPathResult(paths[i], exists);
        if (exists)
            return paths[i];
    }

    return NULL;
}

static void DebugHRESULT(HRESULT hr)
{
    static const char hex[] = "0123456789ABCDEF";
    char message[] = "[M0.4.2a] D3DX PNG decode FAILED. HRESULT=0x00000000\n";
    unsigned int value = (unsigned int)hr;
    const int firstDigit = 47;

    for (int i = 0; i < 8; ++i)
    {
        const int shift = (7 - i) * 4;
        message[firstDigit + i] = hex[(value >> shift) & 0xF];
    }

    OutputDebugStringA(message);
}

static bool LoadPNG128(const char* path)
{
    if (path == NULL)
        return false;

    IDirect3DDevice9* device = GX360_GetDevice();
    if (device == NULL)
        return false;

    OutputDebugStringA("[M0.4.2a] File exists. Trying D3DX PNG decode: ");
    OutputDebugStringA(path);
    OutputDebugStringA("\n");

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
        DebugHRESULT(hr);
        return false;
    }

    D3DLOCKED_RECT locked;
    ZeroMemory(&locked, sizeof(locked));
    hr = sourceTexture->LockRect(0, &locked, NULL, D3DLOCK_READONLY);
    if (FAILED(hr))
    {
        sourceTexture->Release();
        OutputDebugStringA("[M0.4.2a] PNG texture LockRect failed.\n");
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

    OutputDebugStringA("[M0.4.2a] SUCCESS: PNG decoded to 128x128 ARGB pixels.\n");
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
    OutputDebugStringA(" Wii2Xenon Runtime - M0.4.2a\n");
    OutputDebugStringA(" PNG Filesystem Diagnostic\n");
    OutputDebugStringA("============================================\n");

    if (!GX360_Init())
    {
        OutputDebugStringA("[Wii2Xenon] FATAL: GX360 initialization failed!\n");
        for (;;)
            Sleep(1000);
    }

    PAD_Init();
    bool rumbleEnabled = false;

    const char* pngPath = FindPlaceholderPath();
    if (pngPath == NULL)
        OutputDebugStringA("[M0.4.2a] No candidate filesystem path could open placeholder4.png.\n");

    if (!LoadPNG128(pngPath))
    {
        OutputDebugStringA("[M0.4.2a] Using fallback texture. Check FOUND/MISS and HRESULT messages above.\n");
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

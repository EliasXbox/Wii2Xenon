#include <xtl.h>
#include "GX360.h"
#include "GXCompat.h"
#include "WiiXInput.h"

// ============================================================
// Wii2Xenon - M0.4.0
// First GX-style textured quad
// ============================================================

static const GXU16 TEST_TEXTURE_SIZE = 64;
static DWORD g_CheckerTexture[TEST_TEXTURE_SIZE * TEST_TEXTURE_SIZE];

static void BuildCheckerTexture(void)
{
    for (GXU16 y = 0; y < TEST_TEXTURE_SIZE; ++y)
    {
        for (GXU16 x = 0; x < TEST_TEXTURE_SIZE; ++x)
        {
            const bool alternate = (((x / 8) + (y / 8)) & 1) != 0;
            const DWORD colorA = D3DCOLOR_ARGB(255, 255, 255, 255);
            const DWORD colorB = D3DCOLOR_ARGB(255, 55, 120, 255);
            g_CheckerTexture[(y * TEST_TEXTURE_SIZE) + x] = alternate ? colorA : colorB;
        }
    }
}

VOID __cdecl main()
{
    OutputDebugStringA("============================================\n");
    OutputDebugStringA(" Wii2Xenon Runtime - M0.4.0\n");
    OutputDebugStringA(" First GX-style Textured Quad\n");
    OutputDebugStringA("============================================\n");

    if (!GX360_Init())
    {
        OutputDebugStringA("[Wii2Xenon] FATAL: GX360 initialization failed!\n");
        for (;;)
            Sleep(1000);
    }

    PAD_Init();
    bool rumbleEnabled = false;

    BuildCheckerTexture();

    GXTexObj checker;
    ZeroMemory(&checker, sizeof(checker));
    GX_InitTexObj(&checker, g_CheckerTexture,
        TEST_TEXTURE_SIZE, TEST_TEXTURE_SIZE,
        GX_TF_RGBA8, GX_REPEAT, GX_REPEAT, 0);
    GX_LoadTexObj(&checker, GX_TEXMAP0);

    OutputDebugStringA("[Wii2Xenon] Rendering checker texture through GX_TexCoord2f32.\n");
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

        GX_Begin(GX_QUADS, 0, 4);

        GX_Position3f32(-0.70f, -0.70f, 0.0f);
        GX_Color4u8(255, 255, 255, 255);
        GX_TexCoord2f32(0.0f, 2.0f);

        GX_Position3f32(0.70f, -0.70f, 0.0f);
        GX_Color4u8(255, 255, 255, 255);
        GX_TexCoord2f32(2.0f, 2.0f);

        GX_Position3f32(0.70f, 0.70f, 0.0f);
        GX_Color4u8(255, 255, 255, 255);
        GX_TexCoord2f32(2.0f, 0.0f);

        GX_Position3f32(-0.70f, 0.70f, 0.0f);
        GX_Color4u8(255, 255, 255, 255);
        GX_TexCoord2f32(0.0f, 0.0f);

        GX_End();
        GX360_Present();
    }
}

#include <xtl.h>
#include "GX360.h"
#include "GXCompat.h"
#include "WiiXInput.h"

// ============================================================
// Wii2Xenon - M0.4.1
// GX_InitTexObjLOD + NEAR/LINEAR filtering comparison
// ============================================================

static const GXU16 TEST_TEXTURE_SIZE = 16;
static DWORD g_CheckerTexture[TEST_TEXTURE_SIZE * TEST_TEXTURE_SIZE];

static void BuildCheckerTexture(void)
{
    for (GXU16 y = 0; y < TEST_TEXTURE_SIZE; ++y)
    {
        for (GXU16 x = 0; x < TEST_TEXTURE_SIZE; ++x)
        {
            const bool alternate = ((x + y) & 1) != 0;
            const DWORD colorA = D3DCOLOR_ARGB(255, 255, 255, 255);
            const DWORD colorB = D3DCOLOR_ARGB(255, 55, 120, 255);
            g_CheckerTexture[(y * TEST_TEXTURE_SIZE) + x] = alternate ? colorA : colorB;
        }
    }
}

static void DrawTexturedQuad(float left, float right, GXTexObj* texture)
{
    GX_LoadTexObj(texture, GX_TEXMAP0);

    GX_Begin(GX_QUADS, 0, 4);

    GX_Position3f32(left, -0.58f, 0.0f);
    GX_Color4u8(255, 255, 255, 255);
    GX_TexCoord2f32(0.0f, 1.0f);

    GX_Position3f32(right, -0.58f, 0.0f);
    GX_Color4u8(255, 255, 255, 255);
    GX_TexCoord2f32(1.0f, 1.0f);

    GX_Position3f32(right, 0.58f, 0.0f);
    GX_Color4u8(255, 255, 255, 255);
    GX_TexCoord2f32(1.0f, 0.0f);

    GX_Position3f32(left, 0.58f, 0.0f);
    GX_Color4u8(255, 255, 255, 255);
    GX_TexCoord2f32(0.0f, 0.0f);

    GX_End();
}

VOID __cdecl main()
{
    OutputDebugStringA("============================================\n");
    OutputDebugStringA(" Wii2Xenon Runtime - M0.4.1\n");
    OutputDebugStringA(" GX_NEAR vs GX_LINEAR Texture Filter Test\n");
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

    GXTexObj nearTexture;
    GXTexObj linearTexture;
    ZeroMemory(&nearTexture, sizeof(nearTexture));
    ZeroMemory(&linearTexture, sizeof(linearTexture));

    GX_InitTexObj(&nearTexture, g_CheckerTexture,
        TEST_TEXTURE_SIZE, TEST_TEXTURE_SIZE,
        GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, 0);
    GX_InitTexObjLOD(&nearTexture,
        GX_NEAR, GX_NEAR,
        0.0f, 0.0f, 0.0f,
        0, 0, 0);

    GX_InitTexObj(&linearTexture, g_CheckerTexture,
        TEST_TEXTURE_SIZE, TEST_TEXTURE_SIZE,
        GX_TF_RGBA8, GX_CLAMP, GX_CLAMP, 0);
    GX_InitTexObjLOD(&linearTexture,
        GX_LINEAR, GX_LINEAR,
        0.0f, 0.0f, 0.0f,
        0, 0, 0);

    OutputDebugStringA("[Wii2Xenon] Left quad: GX_NEAR (sharp pixels).\n");
    OutputDebugStringA("[Wii2Xenon] Right quad: GX_LINEAR (smoothed pixels).\n");
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

        // Point filtering: enlarged 16x16 texels should stay visibly crisp.
        DrawTexturedQuad(-0.90f, -0.08f, &nearTexture);

        // Bilinear filtering: the same 16x16 texture should look smoother.
        DrawTexturedQuad(0.08f, 0.90f, &linearTexture);

        GX360_Present();
    }
}

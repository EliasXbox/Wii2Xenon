#include <xtl.h>
#include "GX360.h"
#include "GXCompat.h"
#include "WiiXInput.h"

// ============================================================
// Wii2Xenon - M0.3.2
// GX_QUADS + GX_TRIANGLESTRIP regression test
// ============================================================

VOID __cdecl main()
{
    OutputDebugStringA("============================================\n");
    OutputDebugStringA(" Wii2Xenon Runtime - M0.3.2\n");
    OutputDebugStringA(" GX_QUADS + GX_TRIANGLESTRIP Test\n");
    OutputDebugStringA("============================================\n");

    if (!GX360_Init())
    {
        OutputDebugStringA("[Wii2Xenon] FATAL: GX360 initialization failed!\n");
        for (;;)
            Sleep(1000);
    }

    PAD_Init();
    bool rumbleEnabled = false;

    OutputDebugStringA("[Wii2Xenon] Left: GX_QUADS. Right: GX_TRIANGLESTRIP.\n");
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

        // Left: one GX quad. GX360 translates it into two Xenos triangles.
        GX_Begin(GX_QUADS, 0, 4);
        GX_Position3f32(-0.85f, -0.45f, 0.0f); GX_Color4u8(255, 70, 70, 255);
        GX_Position3f32(-0.15f, -0.45f, 0.0f); GX_Color4u8(255, 220, 70, 255);
        GX_Position3f32(-0.15f,  0.45f, 0.0f); GX_Color4u8(70, 255, 120, 255);
        GX_Position3f32(-0.85f,  0.45f, 0.0f); GX_Color4u8(70, 150, 255, 255);
        GX_End();

        // Right: four vertices form two triangles through a triangle strip.
        GX_Begin(GX_TRIANGLESTRIP, 0, 4);
        GX_Position3f32(0.15f, -0.45f, 0.0f); GX_Color4u8(255, 90, 170, 255);
        GX_Position3f32(0.15f,  0.45f, 0.0f); GX_Color4u8(100, 130, 255, 255);
        GX_Position3f32(0.85f, -0.45f, 0.0f); GX_Color4u8(90, 255, 180, 255);
        GX_Position3f32(0.85f,  0.45f, 0.0f); GX_Color4u8(255, 220, 90, 255);
        GX_End();

        GX360_Present();
    }
}

#include <xtl.h>
#include "GX360.h"
#include "GXCompat.h"
#include "WiiXInput.h"

// ============================================================
// Wii2Xenon - M0.3.1
// First GX-style primitive test
// ============================================================

VOID __cdecl main()
{
    OutputDebugStringA("============================================\n");
    OutputDebugStringA(" Wii2Xenon Runtime - M0.3.1\n");
    OutputDebugStringA(" First GX-style Primitive Test\n");
    OutputDebugStringA("============================================\n");

    if (!GX360_Init())
    {
        OutputDebugStringA("[Wii2Xenon] FATAL: GX360 initialization failed!\n");
        for (;;)
            Sleep(1000);
    }

    PAD_Init();
    bool rumbleEnabled = false;

    OutputDebugStringA("[Wii2Xenon] Rendering GX_TRIANGLES through GX360.\n");
    OutputDebugStringA("[Wii2Xenon] Hold A to keep the WiiXInput rumble regression test active.\n");

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

        GX_Begin(GX_TRIANGLES, 0, 3);

        GX_Position3f32(-0.65f, -0.55f, 0.0f);
        GX_Color4u8(255, 64, 64, 255);

        GX_Position3f32(0.65f, -0.55f, 0.0f);
        GX_Color4u8(64, 255, 96, 255);

        GX_Position3f32(0.0f, 0.65f, 0.0f);
        GX_Color4u8(72, 128, 255, 255);

        GX_End();
        GX360_Present();
    }
}

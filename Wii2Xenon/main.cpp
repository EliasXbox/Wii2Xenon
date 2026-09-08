#include <xtl.h>
#include "GX360.h"
#include "WiiXInput.h"

// ============================================================
// Wii2Xenon - M0.3.0
// GX360 core extraction + WiiXInput regression test
// ============================================================

static u8 AxisToColor(s8 value)
{
    int converted = (int)value + 128;

    if (converted < 0)
        converted = 0;
    if (converted > 255)
        converted = 255;

    return (u8)converted;
}

VOID __cdecl main()
{
    OutputDebugStringA("============================================\n");
    OutputDebugStringA(" Wii2Xenon Runtime - M0.3.0\n");
    OutputDebugStringA(" GX360 Core Extraction Test\n");
    OutputDebugStringA("============================================\n");

    if (!GX360_Init())
    {
        OutputDebugStringA("[Wii2Xenon] FATAL: GX360 initialization failed!\n");
        for (;;)
            Sleep(1000);
    }

    PAD_Init();

    OutputDebugStringA("[Wii2Xenon] GX360 is now a standalone module.\n");
    OutputDebugStringA("[Wii2Xenon] Left stick controls red/green.\n");
    OutputDebugStringA("[Wii2Xenon] LT/RT control blue.\n");
    OutputDebugStringA("[Wii2Xenon] Hold X to test the right stick.\n");
    OutputDebugStringA("[Wii2Xenon] Hold A to test PAD rumble.\n");

    bool rumbleEnabled = false;

    for (;;)
    {
        PAD_ScanPads();

        const u16 held = PAD_ButtonsHeld(PAD_CHAN0);

        s8 x = PAD_StickX(PAD_CHAN0);
        s8 y = PAD_StickY(PAD_CHAN0);

        if (held & PAD_BUTTON_X)
        {
            x = PAD_SubStickX(PAD_CHAN0);
            y = PAD_SubStickY(PAD_CHAN0);
        }

        const bool wantsRumble = (held & PAD_BUTTON_A) != 0;

        if (wantsRumble != rumbleEnabled)
        {
            PAD_ControlMotor(
                PAD_CHAN0,
                wantsRumble ? PAD_MOTOR_RUMBLE : PAD_MOTOR_STOP
            );

            rumbleEnabled = wantsRumble;
        }

        const u8 triggerL = PAD_TriggerL(PAD_CHAN0);
        const u8 triggerR = PAD_TriggerR(PAD_CHAN0);

        const u8 red   = AxisToColor(x);
        const u8 green = AxisToColor(y);
        const u8 blue  = (u8)(((u16)triggerL + (u16)triggerR) / 2);

        GX360_Clear(D3DCOLOR_XRGB(red, green, blue));
        GX360_Present();
    }
}

#include <xtl.h>
#include "GX360.h"
#include "WiiXInput.h"
#include "gccore.h"

// P0.0 compile bridge: consume the original 240p controller implementation
// directly from a sibling checkout of EliasXbox/240pTestSuite on the
// wii2xenon/bootstrap branch. This keeps the 240p source in its own repo while
// letting the existing Xbox 360 VS project prove the first real client path.
// Expected checkout layout:
//   parent/Wii2Xenon/
//   parent/240pTestSuite/
extern "C"
{
#include "../../240pTestSuite/240psuite/Wii/240pSuite/source/controller.c"
}

u8 EndProgram = 0;

// ============================================================
// 240p Test Suite on Wii2Xenon - P0.0
// First hybrid build using the real 240p controller.c.
// ============================================================

VOID __cdecl main()
{
    OutputDebugStringA("============================================\n");
    OutputDebugStringA(" 240p Test Suite - Wii2Xenon P0.0\n");
    OutputDebugStringA(" Original 240p controller.c is linked in\n");
    OutputDebugStringA("============================================\n");

    if (!GX360_Init())
    {
        OutputDebugStringA("[240p/Wii2Xenon] FATAL: GX360 initialization failed!\n");
        for (;;)
            Sleep(1000);
    }

    const s32 controllerResult = ControllerInit();
    if (controllerResult < 0)
        OutputDebugStringA("[240p/Wii2Xenon] ControllerInit reported failure.\n");
    else
        OutputDebugStringA("[240p/Wii2Xenon] ControllerInit completed through original 240p code.\n");

    DWORD clearColor = D3DCOLOR_XRGB(24, 28, 40);

    OutputDebugStringA("[240p/Wii2Xenon] P0.0 ready. A/B/X/Y changes the clear color through Controller_ButtonsDown().\n");

    for (;;)
    {
        ControllerScan();
        const u32 pressed = Controller_ButtonsDown(0);

        if (pressed & PAD_BUTTON_A)
            clearColor = D3DCOLOR_XRGB(40, 120, 40);
        else if (pressed & PAD_BUTTON_B)
            clearColor = D3DCOLOR_XRGB(140, 45, 45);
        else if (pressed & PAD_BUTTON_X)
            clearColor = D3DCOLOR_XRGB(45, 75, 150);
        else if (pressed & PAD_BUTTON_Y)
            clearColor = D3DCOLOR_XRGB(145, 120, 35);

        GX360_Clear(clearColor);
        GX360_Present();
    }
}

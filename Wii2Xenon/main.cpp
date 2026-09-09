#include <xtl.h>
#include "GX360.h"
#include "WiiXInput.h"

extern "C" s32 Wii2Xenon_240p_ControllerBootstrap(void);
extern "C" u32 Wii2Xenon_240p_ControllerFrame(void);

// ============================================================
// 240p Test Suite on Wii2Xenon - P0.0
// First hybrid build using real 240p controller code.
// ============================================================

VOID __cdecl main()
{
    OutputDebugStringA("============================================\n");
    OutputDebugStringA(" 240p Test Suite - Wii2Xenon P0.0\n");
    OutputDebugStringA(" Real 240p controller bootstrap\n");
    OutputDebugStringA("============================================\n");

    if (!GX360_Init())
    {
        OutputDebugStringA("[240p/Wii2Xenon] FATAL: GX360 initialization failed!\n");
        for (;;)
            Sleep(1000);
    }

    const s32 controllerResult = Wii2Xenon_240p_ControllerBootstrap();
    if (controllerResult < 0)
        OutputDebugStringA("[240p/Wii2Xenon] ControllerInit reported failure.\n");
    else
        OutputDebugStringA("[240p/Wii2Xenon] ControllerInit completed.\n");

    DWORD clearColor = D3DCOLOR_XRGB(24, 28, 40);

    OutputDebugStringA("[240p/Wii2Xenon] P0.0 ready. Use A/B/X/Y to change the clear color.\n");

    for (;;)
    {
        const u32 pressed = Wii2Xenon_240p_ControllerFrame();

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

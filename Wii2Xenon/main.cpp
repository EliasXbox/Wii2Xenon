#include <xtl.h>
#include "WiiXInput.h"

// ============================================================
// Wii2Xenon - M0.2.3
// GameCube PAD analog + rumble test
// ============================================================

IDirect3D9*       g_pD3D    = NULL;
IDirect3DDevice9* g_pDevice = NULL;

bool GX360_Init()
{
    OutputDebugStringA("[Wii2Xenon/GX360] Initializing Direct3D...\n");

    g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (g_pD3D == NULL)
        return false;

    D3DPRESENT_PARAMETERS params;
    ZeroMemory(&params, sizeof(params));

    params.BackBufferWidth = 1280;
    params.BackBufferHeight = 720;
    params.BackBufferFormat = D3DFMT_A8R8G8B8;
    params.BackBufferCount = 1;
    params.EnableAutoDepthStencil = FALSE;
    params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    HRESULT hr = g_pD3D->CreateDevice(
        D3DADAPTER_DEFAULT,
        D3DDEVTYPE_HAL,
        NULL,
        D3DCREATE_HARDWARE_VERTEXPROCESSING,
        &params,
        &g_pDevice
    );

    if (FAILED(hr))
        return false;

    OutputDebugStringA("[Wii2Xenon/GX360] Direct3D initialized!\n");
    return true;
}

void GX360_Clear(D3DCOLOR frameColor)
{
    if (g_pDevice == NULL)
        return;

    g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, frameColor, 1.0f, 0);
}

void GX360_Present()
{
    if (g_pDevice == NULL)
        return;

    g_pDevice->Present(NULL, NULL, NULL, NULL);
}

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
    OutputDebugStringA(" Wii2Xenon Runtime - M0.2.3\n");
    OutputDebugStringA(" GameCube PAD Analog + Rumble Test\n");
    OutputDebugStringA("============================================\n");

    if (!GX360_Init())
    {
        OutputDebugStringA("[Wii2Xenon] FATAL: Graphics initialization failed!\n");
        for (;;)
            Sleep(1000);
    }

    PAD_Init();

    OutputDebugStringA("[Wii2Xenon] Left stick controls red/green.\n");
    OutputDebugStringA("[Wii2Xenon] LT/RT control blue.\n");
    OutputDebugStringA("[Wii2Xenon] Hold X to test the right stick.\n");
    OutputDebugStringA("[Wii2Xenon] Hold A to test PAD rumble.\n");

    bool rumbleEnabled = false;

    for (;;)
    {
        // One PAD hardware scan per frame.
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

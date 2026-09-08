#include <xtl.h>
#include "WiiXInput.h"

// ============================================================
// Wii2Xenon - M0.2.1
// WiiXInput / WPAD button event test
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

static void LogWPADDown(u32 buttons)
{
    if (buttons & WPAD_BUTTON_A)     OutputDebugStringA("[WiiXInput] DOWN: A\n");
    if (buttons & WPAD_BUTTON_B)     OutputDebugStringA("[WiiXInput] DOWN: B\n");
    if (buttons & WPAD_BUTTON_1)     OutputDebugStringA("[WiiXInput] DOWN: 1\n");
    if (buttons & WPAD_BUTTON_2)     OutputDebugStringA("[WiiXInput] DOWN: 2\n");
    if (buttons & WPAD_BUTTON_PLUS)  OutputDebugStringA("[WiiXInput] DOWN: PLUS\n");
    if (buttons & WPAD_BUTTON_MINUS) OutputDebugStringA("[WiiXInput] DOWN: MINUS\n");
}

static void LogWPADUp(u32 buttons)
{
    if (buttons & WPAD_BUTTON_A)     OutputDebugStringA("[WiiXInput] UP: A\n");
    if (buttons & WPAD_BUTTON_B)     OutputDebugStringA("[WiiXInput] UP: B\n");
    if (buttons & WPAD_BUTTON_1)     OutputDebugStringA("[WiiXInput] UP: 1\n");
    if (buttons & WPAD_BUTTON_2)     OutputDebugStringA("[WiiXInput] UP: 2\n");
    if (buttons & WPAD_BUTTON_PLUS)  OutputDebugStringA("[WiiXInput] UP: PLUS\n");
    if (buttons & WPAD_BUTTON_MINUS) OutputDebugStringA("[WiiXInput] UP: MINUS\n");
}

VOID __cdecl main()
{
    OutputDebugStringA("============================================\n");
    OutputDebugStringA(" Wii2Xenon Runtime - M0.2.1\n");
    OutputDebugStringA(" WPAD Held / Down / Up Test\n");
    OutputDebugStringA("============================================\n");

    if (!GX360_Init())
    {
        OutputDebugStringA("[Wii2Xenon] FATAL: Graphics initialization failed!\n");
        for (;;)
            Sleep(1000);
    }

    if (WPAD_Init() < 0)
        OutputDebugStringA("[Wii2Xenon] WARNING: WPAD_Init returned an error.\n");

    for (;;)
    {
        // Exactly one hardware scan per frame for this test.
        WPAD_ScanPads();

        const u32 held = WPAD_ButtonsHeld(WPAD_CHAN_0);
        const u32 down = WPAD_ButtonsDown(WPAD_CHAN_0);
        const u32 up   = WPAD_ButtonsUp(WPAD_CHAN_0);

        if (down != 0)
            LogWPADDown(down);

        if (up != 0)
            LogWPADUp(up);

        // Held keeps the original visual test intact.
        D3DCOLOR frameColor = D3DCOLOR_XRGB(30, 90, 200);

        if (held & WPAD_BUTTON_A)
            frameColor = D3DCOLOR_XRGB(255, 0, 0);
        else if (held & WPAD_BUTTON_B)
            frameColor = D3DCOLOR_XRGB(0, 255, 0);
        else if (held & WPAD_BUTTON_1)
            frameColor = D3DCOLOR_XRGB(255, 255, 0);
        else if (held & WPAD_BUTTON_2)
            frameColor = D3DCOLOR_XRGB(255, 0, 255);

        GX360_Clear(frameColor);
        GX360_Present();
    }
}

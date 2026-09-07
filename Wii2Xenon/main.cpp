#include <xtl.h>
#include "WiiXInput.h"

// ============================================================
// Wii2Xenon - M0.2
// WiiXInput / WPAD -> XInput Test
// ============================================================


// ============================================================
// Direct3D
// ============================================================

IDirect3D9*       g_pD3D    = NULL;
IDirect3DDevice9* g_pDevice = NULL;


// ============================================================
// Graphics initialization
// ============================================================

bool GX360_Init()
{
    OutputDebugStringA(
        "[Wii2Xenon/GX360] Initializing Direct3D...\n"
    );

    g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);

    if (g_pD3D == NULL)
    {
        OutputDebugStringA(
            "[Wii2Xenon/GX360] ERROR: Direct3DCreate9 failed!\n"
        );

        return false;
    }


    D3DPRESENT_PARAMETERS params;

    ZeroMemory(
        &params,
        sizeof(params)
    );


    params.BackBufferWidth  = 1280;
    params.BackBufferHeight = 720;

    params.BackBufferFormat =
        D3DFMT_A8R8G8B8;

    params.BackBufferCount = 1;

    params.EnableAutoDepthStencil =
        FALSE;

    params.SwapEffect =
        D3DSWAPEFFECT_DISCARD;

    params.PresentationInterval =
        D3DPRESENT_INTERVAL_ONE;


    HRESULT hr =
        g_pD3D->CreateDevice(
            D3DADAPTER_DEFAULT,
            D3DDEVTYPE_HAL,
            NULL,
            D3DCREATE_HARDWARE_VERTEXPROCESSING,
            &params,
            &g_pDevice
        );


    if (FAILED(hr))
    {
        OutputDebugStringA(
            "[Wii2Xenon/GX360] ERROR: CreateDevice failed!\n"
        );

        return false;
    }


    OutputDebugStringA(
        "[Wii2Xenon/GX360] Direct3D initialized!\n"
    );


    return true;
}


// ============================================================
// Clear framebuffer
// ============================================================

void GX360_Clear(D3DCOLOR frameColor)
{
    if (g_pDevice == NULL)
        return;


    g_pDevice->Clear(
        0,
        NULL,
        D3DCLEAR_TARGET,
        frameColor,
        1.0f,
        0
    );
}


// ============================================================
// Present framebuffer
// ============================================================

void GX360_Present()
{
    if (g_pDevice == NULL)
        return;


    g_pDevice->Present(
        NULL,
        NULL,
        NULL,
        NULL
    );
}


// ============================================================
// Xbox 360 entry point
// ============================================================

VOID __cdecl main()
{
    OutputDebugStringA(
        "============================================\n"
    );

    OutputDebugStringA(
        " Wii2Xenon Runtime - M0.2\n"
    );

    OutputDebugStringA(
        " WiiXInput / WPAD -> XInput Test\n"
    );

    OutputDebugStringA(
        "============================================\n"
    );


    // --------------------------------------------------------
    // Graphics
    // --------------------------------------------------------

    if (!GX360_Init())
    {
        OutputDebugStringA(
            "[Wii2Xenon] FATAL: Graphics initialization failed!\n"
        );


        for (;;)
        {
            Sleep(1000);
        }
    }


    // --------------------------------------------------------
    // Wii-style input
    // --------------------------------------------------------

    s32 wpadResult =
        WPAD_Init();


    if (wpadResult < 0)
    {
        OutputDebugStringA(
            "[Wii2Xenon] WARNING: WPAD_Init returned an error.\n"
        );
    }
    else
    {
        OutputDebugStringA(
            "[Wii2Xenon] WPAD initialized!\n"
        );
    }


    OutputDebugStringA(
        "[Wii2Xenon] Entering main loop...\n"
    );


    // ========================================================
    // Main loop
    // ========================================================

    for (;;)
    {
        // Wii API
        WPAD_ScanPads();


        u32 buttons =
            WPAD_ButtonsHeld(
                WPAD_CHAN_0
            );


        // Default Wii2Xenon blue
        D3DCOLOR frameColor =
            D3DCOLOR_XRGB(
                30,
                90,
                200
            );


        // Xbox A -> Wii A -> Red
        if (buttons & WPAD_BUTTON_A)
        {
            frameColor =
                D3DCOLOR_XRGB(
                    255,
                    0,
                    0
                );
        }

        // Xbox B -> Wii B -> Green
        else if (buttons & WPAD_BUTTON_B)
        {
            frameColor =
                D3DCOLOR_XRGB(
                    0,
                    255,
                    0
                );
        }

        // Xbox X -> Wii 1 -> Yellow
        else if (buttons & WPAD_BUTTON_1)
        {
            frameColor =
                D3DCOLOR_XRGB(
                    255,
                    255,
                    0
                );
        }

        // Xbox Y -> Wii 2 -> Magenta
        else if (buttons & WPAD_BUTTON_2)
        {
            frameColor =
                D3DCOLOR_XRGB(
                    255,
                    0,
                    255
                );
        }


        GX360_Clear(
            frameColor
        );


        GX360_Present();
    }
}
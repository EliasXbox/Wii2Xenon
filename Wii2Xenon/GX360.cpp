#include "GX360.h"

// ============================================================
// Wii2Xenon - GX360 core
// ============================================================

static IDirect3D9*       g_pD3D = NULL;
static IDirect3DDevice9* g_pDevice = NULL;

bool GX360_Init(void)
{
    OutputDebugStringA("[Wii2Xenon/GX360] Initializing Direct3D...\n");

    if (g_pDevice != NULL)
        return true;

    g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (g_pD3D == NULL)
    {
        OutputDebugStringA("[Wii2Xenon/GX360] Direct3DCreate9 failed.\n");
        return false;
    }

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
    {
        OutputDebugStringA("[Wii2Xenon/GX360] CreateDevice failed.\n");
        g_pD3D->Release();
        g_pD3D = NULL;
        return false;
    }

    OutputDebugStringA("[Wii2Xenon/GX360] Direct3D initialized.\n");
    return true;
}

void GX360_Shutdown(void)
{
    if (g_pDevice != NULL)
    {
        g_pDevice->Release();
        g_pDevice = NULL;
    }

    if (g_pD3D != NULL)
    {
        g_pD3D->Release();
        g_pD3D = NULL;
    }
}

void GX360_Clear(D3DCOLOR frameColor)
{
    if (g_pDevice == NULL)
        return;

    g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, frameColor, 1.0f, 0);
}

void GX360_Present(void)
{
    if (g_pDevice == NULL)
        return;

    g_pDevice->Present(NULL, NULL, NULL, NULL);
}

IDirect3DDevice9* GX360_GetDevice(void)
{
    return g_pDevice;
}

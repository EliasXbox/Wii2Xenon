#include "GX360.h"
#include "GXCompat.h"
#include <d3dx9.h>

// ============================================================
// Wii2Xenon - GX360 core + first GX-style primitive backend
// ============================================================

static IDirect3D9*                  g_pD3D = NULL;
static IDirect3DDevice9*            g_pDevice = NULL;
static IDirect3DVertexDeclaration9* g_pGXVertexDecl = NULL;
static IDirect3DVertexShader9*      g_pGXVertexShader = NULL;
static IDirect3DPixelShader9*       g_pGXPixelShader = NULL;

struct GX360ImmediateVertex
{
    float x;
    float y;
    float z;
    D3DCOLOR color;
};

static const GXU16 GX360_MAX_IMMEDIATE_VERTICES = 256;
static GX360ImmediateVertex g_GXVertices[GX360_MAX_IMMEDIATE_VERTICES];
static GXU16 g_GXVertexCount = 0;
static GXU16 g_GXExpectedVertices = 0;
static GXU8  g_GXPrimitive = GX_TRIANGLES;
static bool  g_GXHasPendingPosition = false;
static GX360ImmediateVertex g_GXPendingVertex;

static const char g_GXVertexShaderSource[] =
    "struct VS_IN { float3 Pos : POSITION; float4 Color : COLOR0; };\n"
    "struct VS_OUT { float4 Pos : POSITION; float4 Color : COLOR0; };\n"
    "VS_OUT main(VS_IN input) {\n"
    "  VS_OUT output;\n"
    "  output.Pos = float4(input.Pos, 1.0);\n"
    "  output.Color = input.Color;\n"
    "  return output;\n"
    "}\n";

static const char g_GXPixelShaderSource[] =
    "struct PS_IN { float4 Pos : POSITION; float4 Color : COLOR0; };\n"
    "float4 main(PS_IN input) : COLOR0 { return input.Color; }\n";

static void GX360_LogShaderError(ID3DXBuffer* errors)
{
    if (errors != NULL && errors->GetBufferPointer() != NULL)
        OutputDebugStringA((const char*)errors->GetBufferPointer());
}

static bool GX360_InitPrimitivePipeline(void)
{
    if (g_pDevice == NULL)
        return false;

    if (g_pGXVertexDecl != NULL && g_pGXVertexShader != NULL && g_pGXPixelShader != NULL)
        return true;

    OutputDebugStringA("[Wii2Xenon/GX360] Initializing primitive pipeline...\n");

    D3DVERTEXELEMENT9 declaration[] =
    {
        { 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
        D3DDECL_END()
    };

    HRESULT hr = g_pDevice->CreateVertexDeclaration(declaration, &g_pGXVertexDecl);
    if (FAILED(hr))
    {
        OutputDebugStringA("[Wii2Xenon/GX360] CreateVertexDeclaration failed.\n");
        return false;
    }

    ID3DXBuffer* shaderCode = NULL;
    ID3DXBuffer* errors = NULL;

    hr = D3DXCompileShader(g_GXVertexShaderSource, (UINT)(sizeof(g_GXVertexShaderSource) - 1),
        NULL, NULL, "main", "vs_2_0", 0, &shaderCode, &errors, NULL);

    if (FAILED(hr))
    {
        OutputDebugStringA("[Wii2Xenon/GX360] Vertex shader compile failed.\n");
        GX360_LogShaderError(errors);
        if (errors != NULL) errors->Release();
        if (shaderCode != NULL) shaderCode->Release();
        return false;
    }

    if (errors != NULL)
    {
        errors->Release();
        errors = NULL;
    }

    hr = g_pDevice->CreateVertexShader((const DWORD*)shaderCode->GetBufferPointer(), &g_pGXVertexShader);
    shaderCode->Release();
    shaderCode = NULL;

    if (FAILED(hr))
    {
        OutputDebugStringA("[Wii2Xenon/GX360] CreateVertexShader failed.\n");
        return false;
    }

    hr = D3DXCompileShader(g_GXPixelShaderSource, (UINT)(sizeof(g_GXPixelShaderSource) - 1),
        NULL, NULL, "main", "ps_2_0", 0, &shaderCode, &errors, NULL);

    if (FAILED(hr))
    {
        OutputDebugStringA("[Wii2Xenon/GX360] Pixel shader compile failed.\n");
        GX360_LogShaderError(errors);
        if (errors != NULL) errors->Release();
        if (shaderCode != NULL) shaderCode->Release();
        return false;
    }

    if (errors != NULL)
    {
        errors->Release();
        errors = NULL;
    }

    hr = g_pDevice->CreatePixelShader((const DWORD*)shaderCode->GetBufferPointer(), &g_pGXPixelShader);
    shaderCode->Release();

    if (FAILED(hr))
    {
        OutputDebugStringA("[Wii2Xenon/GX360] CreatePixelShader failed.\n");
        return false;
    }

    OutputDebugStringA("[Wii2Xenon/GX360] Primitive pipeline ready.\n");
    return true;
}

bool GX360_Init(void)
{
    OutputDebugStringA("[Wii2Xenon/GX360] Initializing Direct3D...\n");

    if (g_pDevice != NULL)
        return true;

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
    {
        g_pD3D->Release();
        g_pD3D = NULL;
        return false;
    }

    OutputDebugStringA("[Wii2Xenon/GX360] Direct3D initialized.\n");
    return true;
}

void GX360_Shutdown(void)
{
    if (g_pGXPixelShader != NULL)
    {
        g_pGXPixelShader->Release();
        g_pGXPixelShader = NULL;
    }

    if (g_pGXVertexShader != NULL)
    {
        g_pGXVertexShader->Release();
        g_pGXVertexShader = NULL;
    }

    if (g_pGXVertexDecl != NULL)
    {
        g_pGXVertexDecl->Release();
        g_pGXVertexDecl = NULL;
    }

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
    if (g_pDevice != NULL)
        g_pDevice->Clear(0, NULL, D3DCLEAR_TARGET, frameColor, 1.0f, 0);
}

void GX360_Present(void)
{
    if (g_pDevice != NULL)
        g_pDevice->Present(NULL, NULL, NULL, NULL);
}

IDirect3DDevice9* GX360_GetDevice(void)
{
    return g_pDevice;
}

void GX_Begin(GXU8 primitive, GXU8 vtxfmt, GXU16 vertexCount)
{
    (void)vtxfmt;
    g_GXPrimitive = primitive;
    g_GXExpectedVertices = vertexCount;
    g_GXVertexCount = 0;
    g_GXHasPendingPosition = false;
}

void GX_Position3f32(float x, float y, float z)
{
    g_GXPendingVertex.x = x;
    g_GXPendingVertex.y = y;
    g_GXPendingVertex.z = z;
    g_GXPendingVertex.color = D3DCOLOR_ARGB(255, 255, 255, 255);
    g_GXHasPendingPosition = true;
}

void GX_Color4u8(GXU8 r, GXU8 g, GXU8 b, GXU8 a)
{
    if (!g_GXHasPendingPosition || g_GXVertexCount >= GX360_MAX_IMMEDIATE_VERTICES)
        return;

    g_GXPendingVertex.color = D3DCOLOR_ARGB(a, r, g, b);
    g_GXVertices[g_GXVertexCount] = g_GXPendingVertex;
    ++g_GXVertexCount;
    g_GXHasPendingPosition = false;
}

void GX_End(void)
{
    if (g_pDevice == NULL)
        return;

    if (g_GXPrimitive != GX_TRIANGLES)
    {
        OutputDebugStringA("[Wii2Xenon/GX360] GX_End: primitive not implemented yet.\n");
        return;
    }

    if (g_GXVertexCount < 3)
        return;

    if (g_GXExpectedVertices != 0 && g_GXVertexCount != g_GXExpectedVertices)
        OutputDebugStringA("[Wii2Xenon/GX360] GX_End: submitted vertex count differs from GX_Begin count.\n");

    if (!GX360_InitPrimitivePipeline())
        return;

    const UINT primitiveCount = (UINT)(g_GXVertexCount / 3);

    if (SUCCEEDED(g_pDevice->BeginScene()))
    {
        g_pDevice->SetVertexDeclaration(g_pGXVertexDecl);
        g_pDevice->SetVertexShader(g_pGXVertexShader);
        g_pDevice->SetPixelShader(g_pGXPixelShader);
        g_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, primitiveCount,
            g_GXVertices, sizeof(GX360ImmediateVertex));
        g_pDevice->EndScene();
    }
}

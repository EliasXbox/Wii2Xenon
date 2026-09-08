#include "GX360.h"
#include "GXCompat.h"
#include <d3dx9.h>

// ============================================================
// Wii2Xenon - GX360 core + GX-style primitive/texture backend
// M0.4.0: UVs + first linear RGBA8 test texture path
// ============================================================

static IDirect3D9*                  g_pD3D = NULL;
static IDirect3DDevice9*            g_pDevice = NULL;
static IDirect3DVertexDeclaration9* g_pGXVertexDecl = NULL;
static IDirect3DVertexShader9*      g_pGXVertexShader = NULL;
static IDirect3DPixelShader9*       g_pGXPixelShader = NULL;
static IDirect3DTexture9*           g_pWhiteTexture = NULL;
static IDirect3DTexture9*           g_pBoundTexture = NULL;

struct GX360ImmediateVertex
{
    float x;
    float y;
    float z;
    D3DCOLOR color;
    float s;
    float t;
};

static const GXU16 GX360_MAX_IMMEDIATE_VERTICES = 256;
static const GXU16 GX360_MAX_DRAW_VERTICES = 384;
static GX360ImmediateVertex g_GXVertices[GX360_MAX_IMMEDIATE_VERTICES];
static GX360ImmediateVertex g_GXDrawVertices[GX360_MAX_DRAW_VERTICES];
static GXU16 g_GXVertexCount = 0;
static GXU16 g_GXExpectedVertices = 0;
static GXU8  g_GXPrimitive = GX_TRIANGLES;
static bool  g_GXHasPendingVertex = false;
static GX360ImmediateVertex g_GXPendingVertex;

static const char g_GXVertexShaderSource[] =
    "struct VS_IN { float3 Pos : POSITION; float4 Color : COLOR0; float2 Tex : TEXCOORD0; };\n"
    "struct VS_OUT { float4 Pos : POSITION; float4 Color : COLOR0; float2 Tex : TEXCOORD0; };\n"
    "VS_OUT main(VS_IN input) {\n"
    "  VS_OUT output;\n"
    "  output.Pos = float4(input.Pos, 1.0);\n"
    "  output.Color = input.Color;\n"
    "  output.Tex = input.Tex;\n"
    "  return output;\n"
    "}\n";

static const char g_GXPixelShaderSource[] =
    "sampler2D Texture0 : register(s0);\n"
    "struct PS_IN { float4 Pos : POSITION; float4 Color : COLOR0; float2 Tex : TEXCOORD0; };\n"
    "float4 main(PS_IN input) : COLOR0 { return tex2D(Texture0, input.Tex) * input.Color; }\n";

static void GX360_LogShaderError(ID3DXBuffer* errors)
{
    if (errors != NULL && errors->GetBufferPointer() != NULL)
        OutputDebugStringA((const char*)errors->GetBufferPointer());
}

static void GX360_CommitPendingVertex(void)
{
    if (!g_GXHasPendingVertex)
        return;

    if (g_GXVertexCount < GX360_MAX_IMMEDIATE_VERTICES)
        g_GXVertices[g_GXVertexCount++] = g_GXPendingVertex;

    g_GXHasPendingVertex = false;
}

static bool GX360_CreateSolidWhiteTexture(void)
{
    if (g_pDevice == NULL)
        return false;

    if (g_pWhiteTexture != NULL)
        return true;

    HRESULT hr = g_pDevice->CreateTexture(1, 1, 1, 0, D3DFMT_A8R8G8B8,
        D3DPOOL_DEFAULT, &g_pWhiteTexture, NULL);
    if (FAILED(hr) || g_pWhiteTexture == NULL)
        return false;

    D3DLOCKED_RECT locked;
    ZeroMemory(&locked, sizeof(locked));
    hr = g_pWhiteTexture->LockRect(0, &locked, NULL, 0);
    if (FAILED(hr))
        return false;

    *((DWORD*)locked.pBits) = 0xFFFFFFFF;
    g_pWhiteTexture->UnlockRect(0);
    return true;
}

static bool GX360_InitPrimitivePipeline(void)
{
    if (g_pDevice == NULL)
        return false;

    if (g_pGXVertexDecl != NULL && g_pGXVertexShader != NULL &&
        g_pGXPixelShader != NULL && g_pWhiteTexture != NULL)
        return true;

    OutputDebugStringA("[Wii2Xenon/GX360] Initializing textured primitive pipeline...\n");

    D3DVERTEXELEMENT9 declaration[] =
    {
        { 0, 0,  D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
        { 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR,    0 },
        { 0, 16, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
        D3DDECL_END()
    };

    HRESULT hr = g_pDevice->CreateVertexDeclaration(declaration, &g_pGXVertexDecl);
    if (FAILED(hr))
        return false;

    ID3DXBuffer* shaderCode = NULL;
    ID3DXBuffer* errors = NULL;

    hr = D3DXCompileShader(g_GXVertexShaderSource, (UINT)(sizeof(g_GXVertexShaderSource) - 1),
        NULL, NULL, "main", "vs_2_0", 0, &shaderCode, &errors, NULL);
    if (FAILED(hr))
    {
        GX360_LogShaderError(errors);
        if (errors != NULL) errors->Release();
        if (shaderCode != NULL) shaderCode->Release();
        return false;
    }
    if (errors != NULL) { errors->Release(); errors = NULL; }

    hr = g_pDevice->CreateVertexShader((const DWORD*)shaderCode->GetBufferPointer(), &g_pGXVertexShader);
    shaderCode->Release();
    shaderCode = NULL;
    if (FAILED(hr)) return false;

    hr = D3DXCompileShader(g_GXPixelShaderSource, (UINT)(sizeof(g_GXPixelShaderSource) - 1),
        NULL, NULL, "main", "ps_2_0", 0, &shaderCode, &errors, NULL);
    if (FAILED(hr))
    {
        GX360_LogShaderError(errors);
        if (errors != NULL) errors->Release();
        if (shaderCode != NULL) shaderCode->Release();
        return false;
    }
    if (errors != NULL) { errors->Release(); errors = NULL; }

    hr = g_pDevice->CreatePixelShader((const DWORD*)shaderCode->GetBufferPointer(), &g_pGXPixelShader);
    shaderCode->Release();
    if (FAILED(hr)) return false;

    if (!GX360_CreateSolidWhiteTexture())
        return false;

    OutputDebugStringA("[Wii2Xenon/GX360] Textured primitive pipeline ready.\n");
    return true;
}

bool GX360_Init(void)
{
    OutputDebugStringA("[Wii2Xenon/GX360] Initializing Direct3D...\n");
    if (g_pDevice != NULL) return true;

    g_pD3D = Direct3DCreate9(D3D_SDK_VERSION);
    if (g_pD3D == NULL) return false;

    D3DPRESENT_PARAMETERS params;
    ZeroMemory(&params, sizeof(params));
    params.BackBufferWidth = 1280;
    params.BackBufferHeight = 720;
    params.BackBufferFormat = D3DFMT_A8R8G8B8;
    params.BackBufferCount = 1;
    params.EnableAutoDepthStencil = FALSE;
    params.SwapEffect = D3DSWAPEFFECT_DISCARD;
    params.PresentationInterval = D3DPRESENT_INTERVAL_ONE;

    HRESULT hr = g_pD3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, NULL,
        D3DCREATE_HARDWARE_VERTEXPROCESSING, &params, &g_pDevice);
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
    g_pBoundTexture = NULL;
    if (g_pWhiteTexture != NULL) { g_pWhiteTexture->Release(); g_pWhiteTexture = NULL; }
    if (g_pGXPixelShader != NULL) { g_pGXPixelShader->Release(); g_pGXPixelShader = NULL; }
    if (g_pGXVertexShader != NULL) { g_pGXVertexShader->Release(); g_pGXVertexShader = NULL; }
    if (g_pGXVertexDecl != NULL) { g_pGXVertexDecl->Release(); g_pGXVertexDecl = NULL; }
    if (g_pDevice != NULL) { g_pDevice->Release(); g_pDevice = NULL; }
    if (g_pD3D != NULL) { g_pD3D->Release(); g_pD3D = NULL; }
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
    g_GXHasPendingVertex = false;
}

void GX_Position3f32(float x, float y, float z)
{
    GX360_CommitPendingVertex();

    g_GXPendingVertex.x = x;
    g_GXPendingVertex.y = y;
    g_GXPendingVertex.z = z;
    g_GXPendingVertex.color = D3DCOLOR_ARGB(255, 255, 255, 255);
    g_GXPendingVertex.s = 0.0f;
    g_GXPendingVertex.t = 0.0f;
    g_GXHasPendingVertex = true;
}

void GX_Color4u8(GXU8 r, GXU8 g, GXU8 b, GXU8 a)
{
    if (g_GXHasPendingVertex)
        g_GXPendingVertex.color = D3DCOLOR_ARGB(a, r, g, b);
}

void GX_TexCoord2f32(float s, float t)
{
    if (g_GXHasPendingVertex)
    {
        g_GXPendingVertex.s = s;
        g_GXPendingVertex.t = t;
    }
}

void GX_InitTexObj(GXTexObj* obj, const void* imageData, GXU16 width, GXU16 height,
    GXU32 format, GXU8 wrapS, GXU8 wrapT, GXU8 mipmap)
{
    if (obj == NULL)
        return;

    obj->imageData = imageData;
    obj->width = width;
    obj->height = height;
    obj->format = format;
    obj->wrapS = wrapS;
    obj->wrapT = wrapT;
    obj->mipmap = mipmap;
    obj->nativeTexture = NULL;
}

static D3DTEXTUREADDRESS GX360_MapWrapMode(GXU8 mode)
{
    switch (mode)
    {
        case GX_REPEAT: return D3DTADDRESS_WRAP;
        case GX_MIRROR: return D3DTADDRESS_MIRROR;
        case GX_CLAMP:
        default: return D3DTADDRESS_CLAMP;
    }
}

void GX_LoadTexObj(GXTexObj* obj, GXU8 mapId)
{
    if (g_pDevice == NULL || obj == NULL || mapId != GX_TEXMAP0)
        return;

    if (obj->format != GX_TF_RGBA8 || obj->imageData == NULL || obj->width == 0 || obj->height == 0)
    {
        OutputDebugStringA("[Wii2Xenon/GX360] GX_LoadTexObj: unsupported texture object.\n");
        return;
    }

    IDirect3DTexture9* texture = (IDirect3DTexture9*)obj->nativeTexture;
    if (texture == NULL)
    {
        HRESULT hr = g_pDevice->CreateTexture(obj->width, obj->height, 1, 0,
            D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &texture, NULL);
        if (FAILED(hr) || texture == NULL)
        {
            OutputDebugStringA("[Wii2Xenon/GX360] GX_LoadTexObj: CreateTexture failed.\n");
            return;
        }

        D3DLOCKED_RECT locked;
        ZeroMemory(&locked, sizeof(locked));
        hr = texture->LockRect(0, &locked, NULL, 0);
        if (FAILED(hr))
        {
            texture->Release();
            OutputDebugStringA("[Wii2Xenon/GX360] GX_LoadTexObj: LockRect failed.\n");
            return;
        }

        const DWORD* source = (const DWORD*)obj->imageData;
        for (GXU16 y = 0; y < obj->height; ++y)
        {
            DWORD* destination = (DWORD*)((BYTE*)locked.pBits + (y * locked.Pitch));
            for (GXU16 x = 0; x < obj->width; ++x)
                destination[x] = source[(y * obj->width) + x];
        }

        texture->UnlockRect(0);
        obj->nativeTexture = texture;
    }

    g_pBoundTexture = texture;
    g_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, GX360_MapWrapMode(obj->wrapS));
    g_pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, GX360_MapWrapMode(obj->wrapT));
    g_pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
    g_pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
}

static bool GX360_BuildQuadTriangles(GXU16* outVertexCount)
{
    if ((g_GXVertexCount % 4) != 0)
        return false;

    GXU16 dst = 0;
    for (GXU16 src = 0; src < g_GXVertexCount; src += 4)
    {
        if ((GXU16)(dst + 6) > GX360_MAX_DRAW_VERTICES)
            return false;

        g_GXDrawVertices[dst++] = g_GXVertices[src + 0];
        g_GXDrawVertices[dst++] = g_GXVertices[src + 1];
        g_GXDrawVertices[dst++] = g_GXVertices[src + 2];
        g_GXDrawVertices[dst++] = g_GXVertices[src + 0];
        g_GXDrawVertices[dst++] = g_GXVertices[src + 2];
        g_GXDrawVertices[dst++] = g_GXVertices[src + 3];
    }

    *outVertexCount = dst;
    return true;
}

void GX_End(void)
{
    GX360_CommitPendingVertex();

    if (g_pDevice == NULL || g_GXVertexCount < 3)
        return;

    if (g_GXExpectedVertices != 0 && g_GXVertexCount != g_GXExpectedVertices)
        OutputDebugStringA("[Wii2Xenon/GX360] GX_End: submitted vertex count differs from GX_Begin count.\n");

    if (!GX360_InitPrimitivePipeline())
        return;

    D3DPRIMITIVETYPE d3dPrimitive;
    const GX360ImmediateVertex* drawVertices = g_GXVertices;
    GXU16 drawVertexCount = g_GXVertexCount;
    UINT primitiveCount = 0;

    switch (g_GXPrimitive)
    {
        case GX_TRIANGLES:
            d3dPrimitive = D3DPT_TRIANGLELIST;
            primitiveCount = (UINT)(drawVertexCount / 3);
            break;
        case GX_TRIANGLESTRIP:
            d3dPrimitive = D3DPT_TRIANGLESTRIP;
            primitiveCount = (UINT)(drawVertexCount - 2);
            break;
        case GX_QUADS:
            if (!GX360_BuildQuadTriangles(&drawVertexCount))
            {
                OutputDebugStringA("[Wii2Xenon/GX360] GX_End: invalid GX_QUADS vertex count.\n");
                return;
            }
            drawVertices = g_GXDrawVertices;
            d3dPrimitive = D3DPT_TRIANGLELIST;
            primitiveCount = (UINT)(drawVertexCount / 3);
            break;
        default:
            OutputDebugStringA("[Wii2Xenon/GX360] GX_End: primitive not implemented yet.\n");
            return;
    }

    if (SUCCEEDED(g_pDevice->BeginScene()))
    {
        g_pDevice->SetVertexDeclaration(g_pGXVertexDecl);
        g_pDevice->SetVertexShader(g_pGXVertexShader);
        g_pDevice->SetPixelShader(g_pGXPixelShader);
        g_pDevice->SetTexture(0, g_pBoundTexture != NULL ? g_pBoundTexture : g_pWhiteTexture);
        g_pDevice->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
        g_pDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        g_pDevice->DrawPrimitiveUP(d3dPrimitive, primitiveCount,
            drawVertices, sizeof(GX360ImmediateVertex));
        g_pDevice->EndScene();
    }
}

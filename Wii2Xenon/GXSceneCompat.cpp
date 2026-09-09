#include "GX360.h"
#include "GXCompat.h"

void GX_SetViewport(float xOrigin,float yOrigin,float width,float height,float nearZ,float farZ)
{
    (void)xOrigin; (void)yOrigin; (void)width; (void)height; (void)nearZ; (void)farZ;
    // GX360 currently renders through a fixed full-backbuffer viewport.
}

void GX_InvVtxCache(void) {}
void GX_ClearVtxDesc(void) {}
void GX_SetVtxDesc(GXU8 attr,GXU8 type) { (void)attr; (void)type; }
void GX_SetVtxAttrFmt(GXU8 vtxfmt,GXU8 attr,GXU8 compCnt,GXU8 compType,GXU8 frac)
{ (void)vtxfmt; (void)attr; (void)compCnt; (void)compType; (void)frac; }

void GX_SetZMode(GXU8 enable,GXU8 func,GXU8 updateEnable)
{
    (void)func;
    IDirect3DDevice9* d = GX360_GetDevice();
    if (!d) return;
    d->SetRenderState(D3DRS_ZENABLE, enable ? TRUE : FALSE);
    d->SetRenderState(D3DRS_ZWRITEENABLE, updateEnable ? TRUE : FALSE);
}

void GX_SetBlendMode(GXU8 type,GXU8 srcFactor,GXU8 dstFactor,GXU8 op)
{
    (void)srcFactor; (void)dstFactor; (void)op;
    IDirect3DDevice9* d = GX360_GetDevice();
    if (!d) return;
    d->SetRenderState(D3DRS_ALPHABLENDENABLE, type == GX_BM_BLEND ? TRUE : FALSE);
    if (type == GX_BM_BLEND)
    {
        d->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
        d->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
    }
}

void GX_SetAlphaUpdate(GXU8 enable) { (void)enable; }
void GX_SetColorUpdate(GXU8 enable) { (void)enable; }

#pragma once

#include <xtl.h>

// Wii2Xenon - GX360 core
// Xbox 360 graphics backend used by the future Wii GX compatibility layer.

bool GX360_Init(void);
void GX360_Shutdown(void);
void GX360_Clear(D3DCOLOR frameColor);
void GX360_Present(void);
IDirect3DDevice9* GX360_GetDevice(void);

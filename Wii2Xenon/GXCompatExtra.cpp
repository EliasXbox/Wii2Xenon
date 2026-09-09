#include "GXCompat.h"

void GX_Position2f32(float x, float y)
{
    GX_Position3f32(x, y, 0.0f);
}

void GX_DrawDone(void)
{
    // Wii GX waits for queued GPU work here. The current Wii2Xenon immediate
    // path submits during GX_End(), so P0.1 does not need an extra stall yet.
}

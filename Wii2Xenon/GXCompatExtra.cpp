#include "GXCompat.h"

static Mtx44 g_projection;
static bool g_hasOrthoProjection = false;

void guOrtho(Mtx44 m,float top,float bottom,float left,float right,float nearZ,float farZ)
{
    ZeroMemory(m, sizeof(Mtx44));

    // libogc-style orthographic matrix. Keep the matrix itself so future GX
    // clients can inspect/use the same representation; P0.3 consumes the XY
    // terms in GX_Position2f32 before feeding GX360's current clip-space path.
    m[0][0] = 2.0f / (right - left);
    m[0][3] = -(right + left) / (right - left);
    m[1][1] = 2.0f / (top - bottom);
    m[1][3] = -(top + bottom) / (top - bottom);
    m[2][2] = -1.0f / (farZ - nearZ);
    m[2][3] = -nearZ / (farZ - nearZ);
    m[3][3] = 1.0f;
}

void GX_LoadProjectionMtx(const Mtx44 m,GXU8 type)
{
    if (type != GX_ORTHOGRAPHIC)
    {
        g_hasOrthoProjection = false;
        return;
    }

    memcpy(g_projection, m, sizeof(Mtx44));
    g_hasOrthoProjection = true;
}

void GX_Position2f32(float x, float y)
{
    if (g_hasOrthoProjection)
    {
        const float clipX = g_projection[0][0] * x + g_projection[0][3];
        const float clipY = g_projection[1][1] * y + g_projection[1][3];
        GX_Position3f32(clipX, clipY, 0.0f);
        return;
    }

    GX_Position3f32(x, y, 0.0f);
}

void GX_DrawDone(void)
{
    // Wii GX waits for queued GPU work here. The current Wii2Xenon immediate
    // path submits during GX_End(), so an extra stall is not required yet.
}

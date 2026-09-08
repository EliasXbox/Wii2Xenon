#include <xtl.h>
#include <math.h>
#include "GX360.h"
#include "GXCompat.h"
#include "WiiXInput.h"

// ============================================================
// Wii2Xenon - M0.3.1 3D rotation experiment
// Left stick rotates the triangle around X/Y.
// ============================================================

struct Vec3
{
    float x;
    float y;
    float z;
};

static Vec3 RotateXY(const Vec3& v, float angleX, float angleY)
{
    const float sx = sinf(angleX);
    const float cx = cosf(angleX);
    const float sy = sinf(angleY);
    const float cy = cosf(angleY);

    Vec3 rx;
    rx.x = v.x;
    rx.y = v.y * cx - v.z * sx;
    rx.z = v.y * sx + v.z * cx;

    Vec3 ry;
    ry.x = rx.x * cy + rx.z * sy;
    ry.y = rx.y;
    ry.z = -rx.x * sy + rx.z * cy;
    return ry;
}

static Vec3 ProjectPerspective(const Vec3& v)
{
    // Small CPU-side perspective projection for this diagnostic test.
    // The triangle sits in front of a virtual camera at +2.2 units.
    const float cameraDistance = 2.2f;
    float depth = cameraDistance + v.z;
    if (depth < 0.25f)
        depth = 0.25f;

    const float scale = 1.35f / depth;

    Vec3 out;
    out.x = v.x * scale;
    out.y = v.y * scale;
    out.z = 0.0f;
    return out;
}

VOID __cdecl main()
{
    OutputDebugStringA("============================================\n");
    OutputDebugStringA(" Wii2Xenon - GX360 3D Rotation Test\n");
    OutputDebugStringA(" Left stick rotates triangle on X/Y axes\n");
    OutputDebugStringA("============================================\n");

    if (!GX360_Init())
    {
        OutputDebugStringA("[Wii2Xenon] FATAL: GX360 initialization failed!\n");
        for (;;)
            Sleep(1000);
    }

    PAD_Init();

    float angleX = 0.0f;
    float angleY = 0.0f;
    bool rumbleEnabled = false;

    const Vec3 baseVertices[3] =
    {
        { -0.85f, -0.70f, 0.0f },
        {  0.85f, -0.70f, 0.0f },
        {  0.00f,  0.85f, 0.0f }
    };

    for (;;)
    {
        PAD_ScanPads();

        const s8 stickX = PAD_StickX(PAD_CHAN0);
        const s8 stickY = PAD_StickY(PAD_CHAN0);
        const u16 held = PAD_ButtonsHeld(PAD_CHAN0);

        const float deadzone = 12.0f;
        if ((float)stickX > deadzone || (float)stickX < -deadzone)
            angleY += ((float)stickX / 127.0f) * 0.035f;

        if ((float)stickY > deadzone || (float)stickY < -deadzone)
            angleX -= ((float)stickY / 127.0f) * 0.035f;

        const bool wantsRumble = (held & PAD_BUTTON_A) != 0;
        if (wantsRumble != rumbleEnabled)
        {
            PAD_ControlMotor(PAD_CHAN0,
                wantsRumble ? PAD_MOTOR_RUMBLE : PAD_MOTOR_STOP);
            rumbleEnabled = wantsRumble;
        }

        Vec3 p0 = ProjectPerspective(RotateXY(baseVertices[0], angleX, angleY));
        Vec3 p1 = ProjectPerspective(RotateXY(baseVertices[1], angleX, angleY));
        Vec3 p2 = ProjectPerspective(RotateXY(baseVertices[2], angleX, angleY));

        GX360_Clear(D3DCOLOR_XRGB(24, 28, 40));

        GX_Begin(GX_TRIANGLES, 0, 3);
        GX_Position3f32(p0.x, p0.y, p0.z); GX_Color4u8(255, 64, 64, 255);
        GX_Position3f32(p1.x, p1.y, p1.z); GX_Color4u8(64, 255, 96, 255);
        GX_Position3f32(p2.x, p2.y, p2.z); GX_Color4u8(72, 128, 255, 255);
        GX_End();

        GX360_Present();
    }
}

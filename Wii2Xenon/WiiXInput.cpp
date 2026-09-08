#include "WiiXInput.h"

// ============================================================
// Internal state
// ============================================================

static XINPUT_STATE g_current[4];
static XINPUT_STATE g_previous[4];
static bool g_connected[4];

// ============================================================
// Internal helpers
// ============================================================

static bool WiiXInput_ValidChannel(int channel)
{
    return channel >= 0 && channel < 4;
}

static void WiiXInput_ClearStates()
{
    ZeroMemory(g_current, sizeof(g_current));
    ZeroMemory(g_previous, sizeof(g_previous));
    ZeroMemory(g_connected, sizeof(g_connected));
}

static s8 WiiXInput_StickToS8(SHORT value)
{
    return (s8)(value / 256);
}

static DWORD WiiXInput_SetRumble(int channel, bool enabled)
{
    if (!WiiXInput_ValidChannel(channel))
        return ERROR_BAD_ARGUMENTS;

    XINPUT_VIBRATION vibration;
    ZeroMemory(&vibration, sizeof(vibration));

    if (enabled)
    {
        vibration.wLeftMotorSpeed = 0xFFFF;
        vibration.wRightMotorSpeed = 0xFFFF;
    }

    return XInputSetState((DWORD)channel, &vibration);
}

// ------------------------------------------------------------
// Xbox -> GameCube PAD mapping
// ------------------------------------------------------------

static u16 WiiXInput_MapPAD(const XINPUT_GAMEPAD& pad)
{
    u16 buttons = 0;

    if (pad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
        buttons |= PAD_BUTTON_LEFT;
    if (pad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
        buttons |= PAD_BUTTON_RIGHT;
    if (pad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
        buttons |= PAD_BUTTON_DOWN;
    if (pad.wButtons & XINPUT_GAMEPAD_DPAD_UP)
        buttons |= PAD_BUTTON_UP;

    if (pad.wButtons & XINPUT_GAMEPAD_A)
        buttons |= PAD_BUTTON_A;
    if (pad.wButtons & XINPUT_GAMEPAD_B)
        buttons |= PAD_BUTTON_B;
    if (pad.wButtons & XINPUT_GAMEPAD_X)
        buttons |= PAD_BUTTON_X;
    if (pad.wButtons & XINPUT_GAMEPAD_Y)
        buttons |= PAD_BUTTON_Y;

    if (pad.wButtons & XINPUT_GAMEPAD_START)
        buttons |= PAD_BUTTON_START;
    if (pad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
        buttons |= PAD_TRIGGER_L;
    if (pad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
        buttons |= PAD_TRIGGER_R;

    return buttons;
}

// ------------------------------------------------------------
// Xbox -> Wii Remote mapping
// ------------------------------------------------------------

static u32 WiiXInput_MapWPAD(const XINPUT_GAMEPAD& pad)
{
    u32 buttons = 0;

    if (pad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT)
        buttons |= WPAD_BUTTON_LEFT;
    if (pad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT)
        buttons |= WPAD_BUTTON_RIGHT;
    if (pad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN)
        buttons |= WPAD_BUTTON_DOWN;
    if (pad.wButtons & XINPUT_GAMEPAD_DPAD_UP)
        buttons |= WPAD_BUTTON_UP;

    if (pad.wButtons & XINPUT_GAMEPAD_A)
        buttons |= WPAD_BUTTON_A;
    if (pad.wButtons & XINPUT_GAMEPAD_B)
        buttons |= WPAD_BUTTON_B;

    if (pad.wButtons & XINPUT_GAMEPAD_X)
        buttons |= WPAD_BUTTON_1;
    if (pad.wButtons & XINPUT_GAMEPAD_Y)
        buttons |= WPAD_BUTTON_2;

    if (pad.wButtons & XINPUT_GAMEPAD_START)
        buttons |= WPAD_BUTTON_PLUS;
    if (pad.wButtons & XINPUT_GAMEPAD_BACK)
        buttons |= WPAD_BUTTON_MINUS;

    return buttons;
}

// ============================================================
// Scan
// ============================================================

static u32 WiiXInput_Scan()
{
    u32 connectedMask = 0;

    for (DWORD i = 0; i < 4; ++i)
    {
        g_previous[i] = g_current[i];

        XINPUT_STATE newState;
        ZeroMemory(&newState, sizeof(newState));

        DWORD result = XInputGetState(i, &newState);

        if (result == ERROR_SUCCESS)
        {
            g_current[i] = newState;
            g_connected[i] = true;
            connectedMask |= (1u << i);
        }
        else
        {
            ZeroMemory(&g_current[i], sizeof(g_current[i]));
            g_connected[i] = false;
        }
    }

    return connectedMask;
}

// ============================================================
// PAD implementation
// ============================================================

u32 PAD_Init(void)
{
    WiiXInput_ClearStates();
    OutputDebugStringA("[Wii2Xenon/WiiXInput] PAD initialized.\n");
    return 1;
}

u32 PAD_ScanPads(void)
{
    return WiiXInput_Scan();
}

u16 PAD_ButtonsHeld(int pad)
{
    if (!WiiXInput_ValidChannel(pad) || !g_connected[pad])
        return 0;

    return WiiXInput_MapPAD(g_current[pad].Gamepad);
}

u16 PAD_ButtonsDown(int pad)
{
    if (!WiiXInput_ValidChannel(pad) || !g_connected[pad])
        return 0;

    u16 current = WiiXInput_MapPAD(g_current[pad].Gamepad);
    u16 previous = WiiXInput_MapPAD(g_previous[pad].Gamepad);
    return current & ~previous;
}

u16 PAD_ButtonsUp(int pad)
{
    if (!WiiXInput_ValidChannel(pad))
        return 0;

    u16 current = WiiXInput_MapPAD(g_current[pad].Gamepad);
    u16 previous = WiiXInput_MapPAD(g_previous[pad].Gamepad);
    return previous & ~current;
}

s8 PAD_StickX(int pad)
{
    if (!WiiXInput_ValidChannel(pad) || !g_connected[pad])
        return 0;

    return WiiXInput_StickToS8(g_current[pad].Gamepad.sThumbLX);
}

s8 PAD_StickY(int pad)
{
    if (!WiiXInput_ValidChannel(pad) || !g_connected[pad])
        return 0;

    return WiiXInput_StickToS8(g_current[pad].Gamepad.sThumbLY);
}

s8 PAD_SubStickX(int pad)
{
    if (!WiiXInput_ValidChannel(pad) || !g_connected[pad])
        return 0;

    return WiiXInput_StickToS8(g_current[pad].Gamepad.sThumbRX);
}

s8 PAD_SubStickY(int pad)
{
    if (!WiiXInput_ValidChannel(pad) || !g_connected[pad])
        return 0;

    return WiiXInput_StickToS8(g_current[pad].Gamepad.sThumbRY);
}

u8 PAD_TriggerL(int pad)
{
    if (!WiiXInput_ValidChannel(pad) || !g_connected[pad])
        return 0;

    return (u8)g_current[pad].Gamepad.bLeftTrigger;
}

u8 PAD_TriggerR(int pad)
{
    if (!WiiXInput_ValidChannel(pad) || !g_connected[pad])
        return 0;

    return (u8)g_current[pad].Gamepad.bRightTrigger;
}

void PAD_ControlMotor(s32 chan, u32 cmd)
{
    if (!WiiXInput_ValidChannel(chan))
        return;

    const bool enabled = (cmd == PAD_MOTOR_RUMBLE);
    WiiXInput_SetRumble(chan, enabled);
}

// ============================================================
// WPAD implementation
// ============================================================

s32 WPAD_Init(void)
{
    WiiXInput_ClearStates();
    OutputDebugStringA("[Wii2Xenon/WiiXInput] WPAD initialized.\n");
    return 0;
}

s32 WPAD_ScanPads(void)
{
    WiiXInput_Scan();
    return 0;
}

u32 WPAD_ButtonsHeld(int chan)
{
    if (!WiiXInput_ValidChannel(chan) || !g_connected[chan])
        return 0;

    return WiiXInput_MapWPAD(g_current[chan].Gamepad);
}

u32 WPAD_ButtonsDown(int chan)
{
    if (!WiiXInput_ValidChannel(chan) || !g_connected[chan])
        return 0;

    u32 current = WiiXInput_MapWPAD(g_current[chan].Gamepad);
    u32 previous = WiiXInput_MapWPAD(g_previous[chan].Gamepad);
    return current & ~previous;
}

u32 WPAD_ButtonsUp(int chan)
{
    if (!WiiXInput_ValidChannel(chan))
        return 0;

    u32 current = WiiXInput_MapWPAD(g_current[chan].Gamepad);
    u32 previous = WiiXInput_MapWPAD(g_previous[chan].Gamepad);
    return previous & ~current;
}

s32 WPAD_Rumble(s32 chan, int status)
{
    if (!WiiXInput_ValidChannel(chan))
        return -1;

    DWORD result = WiiXInput_SetRumble(chan, status != 0);
    return (result == ERROR_SUCCESS) ? 0 : -1;
}

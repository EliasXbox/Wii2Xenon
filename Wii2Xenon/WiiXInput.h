#pragma once

#include <xtl.h>

// ============================================================
// Wii2Xenon - WiiXInput
// Initial libogc-compatible input layer
// ============================================================

typedef unsigned char      u8;
typedef signed char        s8;
typedef unsigned short     u16;
typedef signed short       s16;
typedef unsigned int       u32;
typedef signed int         s32;

// ------------------------------------------------------------
// PAD - GameCube Controller compatibility
// ------------------------------------------------------------

#define PAD_CHAN0       0
#define PAD_CHAN1       1
#define PAD_CHAN2       2
#define PAD_CHAN3       3
#define PAD_CHANMAX     4

#define PAD_BUTTON_LEFT     0x0001
#define PAD_BUTTON_RIGHT    0x0002
#define PAD_BUTTON_DOWN     0x0004
#define PAD_BUTTON_UP       0x0008

#define PAD_TRIGGER_Z       0x0010
#define PAD_TRIGGER_R       0x0020
#define PAD_TRIGGER_L       0x0040

#define PAD_BUTTON_A        0x0100
#define PAD_BUTTON_B        0x0200
#define PAD_BUTTON_X        0x0400
#define PAD_BUTTON_Y        0x0800
#define PAD_BUTTON_MENU     0x1000
#define PAD_BUTTON_START    0x1000

// ------------------------------------------------------------
// WPAD - Wii Remote compatibility
// ------------------------------------------------------------

enum
{
    WPAD_CHAN_ALL = -1,
    WPAD_CHAN_0,
    WPAD_CHAN_1,
    WPAD_CHAN_2,
    WPAD_CHAN_3
};

#define WPAD_BUTTON_2       0x0001
#define WPAD_BUTTON_1       0x0002
#define WPAD_BUTTON_B       0x0004
#define WPAD_BUTTON_A       0x0008
#define WPAD_BUTTON_MINUS   0x0010
#define WPAD_BUTTON_HOME    0x0080

#define WPAD_BUTTON_LEFT    0x0100
#define WPAD_BUTTON_RIGHT   0x0200
#define WPAD_BUTTON_DOWN    0x0400
#define WPAD_BUTTON_UP      0x0800

#define WPAD_BUTTON_PLUS    0x1000

// ------------------------------------------------------------
// PAD API
// ------------------------------------------------------------

u32 PAD_Init(void);
u32 PAD_ScanPads(void);

u16 PAD_ButtonsUp(int pad);
u16 PAD_ButtonsDown(int pad);
u16 PAD_ButtonsHeld(int pad);

s8 PAD_StickX(int pad);
s8 PAD_StickY(int pad);
s8 PAD_SubStickX(int pad);
s8 PAD_SubStickY(int pad);
u8 PAD_TriggerL(int pad);
u8 PAD_TriggerR(int pad);

// ------------------------------------------------------------
// WPAD API
// ------------------------------------------------------------

s32 WPAD_Init(void);
s32 WPAD_ScanPads(void);

u32 WPAD_ButtonsUp(int chan);
u32 WPAD_ButtonsDown(int chan);
u32 WPAD_ButtonsHeld(int chan);

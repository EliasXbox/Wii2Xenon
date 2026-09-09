#pragma once

// Wii2Xenon bootstrap compatibility shim for source that expects libogc's
// <gccore.h>. Keep this intentionally small: only expose pieces that are
// already backed by Wii2Xenon, and grow it when real clients require more.

#include "WiiXInput.h"
#include "GXCompat.h"

// Common libogc scalar aliases used by the 240p Test Suite.
typedef float         f32;
typedef double        f64;
typedef unsigned long ulong;
typedef float         Mtx[3][4];
typedef float         Mtx44[4][4];

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef GX_FALSE
#define GX_FALSE 0
#endif

// Early bootstrap note:
// - PAD_* is provided by WiiXInput.
// - the currently implemented GX_* subset is provided by GXCompat.
// - video/system/cache APIs are deliberately NOT faked here yet.
//   Missing symbols should tell us what the next compatibility slice is.

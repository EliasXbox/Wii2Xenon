# Wii2Xenon (Wii360)

Wii2Xenon is an experimental compatibility/porting layer for bringing Wii and GameCube software concepts and APIs to the Xbox 360.

## Branch

**`feature/gx360-primitives` — M0.3.1 first GX-style primitive**

This branch is the first step from a generic Xbox 360 graphics backend toward an actual GX compatibility surface.

## What this branch adds

- minimal `GXCompat.h` interface
- `GX_Begin()`
- `GX_Position3f32()`
- `GX_Color4u8()`
- `GX_End()`
- immediate vertex collection for `GX_TRIANGLES`
- a small shader-based Direct3D pipeline created by GX360
- a three-vertex RGB triangle regression test in `main.cpp`
- WiiXInput rumble remains active as a regression check

```text
Wii / GameCube-style GX calls
          |
      GXCompat
          |
       GX360
          |
 Direct3D 9+ / Xenos
          |
       Xbox 360
```

## Test target

**Status: 🧪 ready for VS2010 / Xenia / real Xbox 360 validation**

Expected result:

- dark background
- one large triangle near the center of the screen
- red-ish left vertex
- green-ish right vertex
- blue-ish top vertex
- colors interpolate across the triangle
- holding A still activates the controller rumble regression test

If the project builds but the triangle is missing, check the debug output for `GX360` shader/pipeline messages.

## Current milestones

```text
M0.0   XEX build / boot                    ✅
M0.1   Direct3D Clear / Present            ✅
M0.2   WiiXInput digital input             ✅
M0.2.2 PAD analog / triggers               ✅
M0.2.3 PAD / WPAD rumble                  ✅
M0.3.0 Standalone GX360 core              ✅ Xenia + Xbox 360
M0.3.1 First GX-style triangle            🧪
```

## Why this also matters for GameCube

The Wii GX API is descended from the GameCube graphics API, so a carefully designed GX360 layer can eventually serve both Wii and GameCube ports. The plan is to grow the compatibility surface from real client needs instead of trying to implement the entire GX/TEV feature set at once.

A future GameCube client such as a legal, open-source decompilation can therefore reuse the same GX360 direction, while platform-specific OS/input/audio compatibility lives in separate layers.

## Development setup

- Visual Studio 2010 + Xbox 360 XDK
- Xenia and/or Xbox 360 hardware
- libogc used as a Wii/GameCube API reference

## Next direction

After the first triangle is validated, add additional primitive modes and begin texture support required by the 240p Test Suite and later Wii/GameCube clients.

## Legal note

Wii2Xenon does not include Nintendo game assets, ROMs, ISOs, or proprietary SDK files. Development should use legally obtained software/data and open-source or independently written compatibility code.

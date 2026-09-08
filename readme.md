# Wii2Xenon (Wii360)

Wii2Xenon is an experimental compatibility/porting layer for bringing Wii software concepts and APIs to the Xbox 360.

## Branch

**`feature/gx360-core` — M0.3.0 standalone graphics backend**

This branch starts the GX360 milestone by moving the already-tested Xbox 360 Direct3D setup out of `main.cpp` and into a dedicated graphics module.

It is stacked on top of the tested WiiXInput rumble branch so digital input, analog input, triggers, and rumble remain available as regression checks while graphics work begins.

## What this branch adds

- `GX360.h` public graphics-backend interface
- `GX360.cpp` Direct3D/Xenos backend implementation
- `GX360_Init()`
- `GX360_Shutdown()`
- `GX360_Clear()`
- `GX360_Present()`
- `GX360_GetDevice()` for later low-level GX compatibility work
- a smaller `main.cpp` that consumes GX360 instead of owning Direct3D state
- Visual Studio project entries for the new GX360 source/header files

```text
Wii-style application code
        |
   future GX API
        |
      GX360
        |
 Direct3D 9 / Xenos
        |
     Xbox 360
```

## Test result

**Status: ✅ validated on Xenia and real Xbox 360 hardware**

The regression test behaved identically on the emulator and on real Xbox 360 hardware:

- analog sticks changed framebuffer colors correctly
- LT/RT controlled the blue channel
- holding X switched to the right stick
- holding A enabled controller rumble
- GX360 initialization, clear, and present remained stable after being moved out of `main.cpp`

This validates the M0.3.0 module split and gives the project a stable graphics backend to build the Wii-style compatibility layer on top of.

## Current milestones

```text
M0.0  XEX build / boot                    ✅
M0.1  Direct3D Clear / Present            ✅
M0.2  WiiXInput digital input             ✅
M0.2.2 PAD analog / triggers              ✅
M0.2.3 PAD / WPAD rumble                  ✅
M0.3.0 Standalone GX360 core              ✅ Xenia + Xbox 360
M0.3.1 First GX-style primitive           next
```

## Development setup

- Visual Studio 2010 + Xbox 360 XDK
- Xenia and/or Xbox 360 hardware
- libogc used as the Wii/GameCube API reference

## Next direction

The next step is **M0.3.1 — first GX-style primitive**. The goal is to begin exposing a very small Wii-like graphics API (`GX_Begin`, position/color submission, `GX_End`) while GX360 translates that work to the Xbox 360 graphics backend.

We will implement only the subset required by early test clients instead of trying to reproduce the whole Wii GX/TEV stack at once.

## Legal note

Wii2Xenon does not include Nintendo game assets, ROMs, ISOs, or proprietary SDK files. Development should use legally obtained software/data and open-source or independently written compatibility code.

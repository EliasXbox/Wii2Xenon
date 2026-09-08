# Wii2Xenon (Wii360)

Wii2Xenon is an experimental compatibility/porting layer for bringing Wii software concepts and APIs to the Xbox 360.

The project is currently focused on building the runtime incrementally instead of attempting a full game port all at once.

## Current status

- M0.0 — Minimal Xbox 360 XEX builds and boots: ✅
- M0.1 — Direct3D framebuffer clear + Present: ✅
- M0.2 — Basic WiiXInput (`WPAD_*` -> XInput): ✅
- Next — expand WiiXInput (`PAD_*`, analog input, rumble) and begin GX360 graphics compatibility

## Current architecture

```text
Wii-style application code
        |
        +--> WiiXInput --> XInput
        |
        +--> GX360 ------> Direct3D 9 / Xenos
        |
        +--> Runtime ----> Xbox 360 APIs
```

## Development setup

### Xbox 360 side

- Visual Studio 2010
- Xbox 360 XDK
- Xenia and/or Xbox 360 hardware for testing

### Wii reference side

- devkitPro
- devkitPPC
- libogc

libogc is used as an API/reference target for compatibility work. Wii2Xenon is not intended to require the Wii toolchain when building the Xbox 360 target.

## Repository layout

```text
Wii2Xenon/
├── main.cpp
├── WiiXInput.h
├── WiiXInput.cpp
└── Wii2Xenon.vcxproj
```

The layout will expand as GX360 and runtime modules are introduced.

## Project state

Early work in progress. APIs and structure may change frequently while compatibility layers are being implemented and tested.

## Legal note

Wii2Xenon does not include Nintendo game assets, ROMs, ISOs, or proprietary SDK files. Development should use legally obtained software/data and open-source or independently written compatibility code.

# Wii2Xenon (Wii360)

Wii2Xenon is an experimental compatibility/porting layer for bringing Wii software concepts and APIs to the Xbox 360.

The project is being built incrementally as a reusable runtime/compatibility layer rather than as a one-off port of a single game.

## Branch

**`main` — stable project baseline**

This branch is the primary integration target. Experimental work is developed and tested in feature branches before being folded back into the main project.

## Current status

- M0.0 — Minimal Xbox 360 XEX builds and boots: ✅
- M0.1 — Direct3D framebuffer clear + Present: ✅
- M0.2 — Basic WiiXInput (`WPAD_*` / `PAD_*` -> XInput): ✅
- M0.2.1 — Button event semantics (`Held` / `Down` / `Up`): implemented / test branch
- M0.2.2 — GameCube PAD analog sticks + triggers: ✅ tested
- M0.2.3 — PAD/WPAD rumble -> Xbox 360 vibration: ✅ tested
- Next — M0.3: extract GX360 from `main.cpp` and begin graphics compatibility primitives

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

## Active branches

- `cleanup/repo-baseline` — repository cleanup/documentation baseline
- `feature/wiixinput-events` — Wii-style button press/hold/release events
- `feature/wiixinput-analog` — GameCube PAD analog sticks and triggers
- `feature/wiixinput-rumble` — Wii/GameCube rumble mapped to Xbox 360 vibration

Each branch has its own README variant describing its purpose and test target.

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

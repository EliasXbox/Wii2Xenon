# Wii2Xenon (Wii360)

Wii2Xenon is an experimental compatibility/porting layer for bringing Wii software concepts and APIs to the Xbox 360.

## Branch

**`feature/wiixinput-rumble` — M0.2.3 Wii/GameCube rumble compatibility**

This branch adds vibration output to WiiXInput so Wii/GameCube-style rumble calls can drive an Xbox 360 controller through XInput.

It is stacked on top of the tested analog-input branch.

## What this branch adds

- `PAD_MOTOR_STOP`
- `PAD_MOTOR_RUMBLE`
- `PAD_MOTOR_STOP_HARD`
- `PAD_ControlMotor()` -> Xbox 360 `XInputSetState()` vibration
- `WPAD_Rumble()` -> same vibration backend
- simple hold/release test for motor start and stop

## Test result

**Status: ✅ tested successfully**

The test was run with an original Xbox 360 controller. Holding the mapped PAD A button enabled vibration and releasing it stopped vibration correctly.

```text
Wii / GameCube API
        |
 PAD_ControlMotor()
    WPAD_Rumble()
        |
    WiiXInput
        |
  XInputSetState()
        |
Xbox 360 controller vibration
```

The M0.2.2 analog test remains available in this branch as a regression check.

## Current WiiXInput milestone

```text
Digital buttons       ✅
Analog left stick     ✅
Analog right stick    ✅
Analog triggers       ✅
PAD rumble            ✅
WPAD rumble backend   ✅
```

## Development setup

- Visual Studio 2010 + Xbox 360 XDK
- Xenia and/or Xbox 360 hardware
- libogc used as the Wii/GameCube API reference

## Next direction

After WiiXInput, the next major milestone is **M0.3 — GX360**: extracting graphics code from `main.cpp` and beginning a Wii GX-style graphics compatibility layer over Xbox 360 Direct3D/Xenos.

## Legal note

Wii2Xenon does not include Nintendo game assets, ROMs, ISOs, or proprietary SDK files. Development should use legally obtained software/data and open-source or independently written compatibility code.

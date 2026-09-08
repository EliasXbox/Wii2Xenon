# Wii2Xenon (Wii360)

Wii2Xenon is an experimental compatibility/porting layer for bringing Wii software concepts and APIs to the Xbox 360.

## Branch

**`feature/wiixinput-analog` — M0.2.2 GameCube PAD analog input**

This branch expands WiiXInput beyond digital buttons by mapping Xbox 360 analog controls to libogc-style GameCube PAD APIs.

## What this branch adds

- `PAD_StickX()` / `PAD_StickY()` -> Xbox left stick
- `PAD_SubStickX()` / `PAD_SubStickY()` -> Xbox right stick
- `PAD_TriggerL()` / `PAD_TriggerR()` -> Xbox analog triggers
- signed stick conversion to the libogc-style `s8` range
- visual framebuffer test for analog values

## Test result

**Status: ✅ tested successfully**

The test was run with an original Xbox 360 controller. Analog stick movement and trigger values correctly reached Wii-style PAD functions through WiiXInput.

Test visualization:

```text
Left stick X -> Red
Left stick Y -> Green
LT + RT      -> Blue
Hold X       -> visualize right stick instead
```

## Architecture under test

```text
Original Xbox 360 Controller
          |
        XInput
          |
      WiiXInput
          |
  GameCube PAD-style API
     /      |       \
 sticks   substick  triggers
```

## Development setup

- Visual Studio 2010 + Xbox 360 XDK
- Xenia and/or Xbox 360 hardware
- libogc used as the Wii/GameCube API reference

## Project state

M0.2.2 is considered functionally validated for the implemented analog subset. Rumble work continues in `feature/wiixinput-rumble`.

## Legal note

Wii2Xenon does not include Nintendo game assets, ROMs, ISOs, or proprietary SDK files. Development should use legally obtained software/data and open-source or independently written compatibility code.

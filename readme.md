# Wii2Xenon (Wii360)

Wii2Xenon is an experimental compatibility/porting layer for bringing Wii software concepts and APIs to the Xbox 360.

## Branch

**`feature/wiixinput-events` — M0.2.1 button event semantics**

This branch focuses on making Wii-style input events behave correctly on top of Xbox 360 XInput.

The important distinction is between a button being held continuously and the one-frame transitions produced when it is pressed or released.

## Focus of this branch

- `WPAD_ButtonsHeld()`
- `WPAD_ButtonsDown()`
- `WPAD_ButtonsUp()`
- one hardware scan per frame
- debug logging for press/release transitions
- preservation of the original M0.2 color-button test

Expected behavior:

```text
Press button   -> Down once + Held
Keep holding   -> Held only
Release button -> Up once
```

This behavior is important for Wii software menus and gameplay logic where a single press must not repeat every frame.

## Architecture under test

```text
Xbox 360 Controller
        |
      XInput
        |
    WiiXInput
        |
 WPAD_ButtonsHeld
 WPAD_ButtonsDown
 WPAD_ButtonsUp
```

## Development setup

- Visual Studio 2010 + Xbox 360 XDK
- Xenia and/or Xbox 360 hardware
- libogc used as the Wii API reference

## Project state

M0.2.1 test branch. Changes here are intentionally focused on digital button-event behavior so they can be validated independently from analog input and rumble work.

## Legal note

Wii2Xenon does not include Nintendo game assets, ROMs, ISOs, or proprietary SDK files. Development should use legally obtained software/data and open-source or independently written compatibility code.

# Wii2Xenon (Wii360)

Wii2Xenon is an experimental compatibility/porting layer for bringing Wii software concepts and APIs to the Xbox 360.

## Branch

**`cleanup/repo-baseline` — repository cleanup baseline**

This branch is focused on repository hygiene and project presentation rather than runtime features.

Its purpose is to keep the source tree easier to clone, review, and maintain by removing generated Visual Studio files from version control, adding ignore rules, and improving the project documentation.

## What this branch contains

- `.gitignore` rules for Visual Studio/Xbox 360 build artifacts
- removal of generated IntelliSense/user-specific files from version control
- expanded project README and legal note
- no intended runtime behavior changes

## Project architecture

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

libogc is used as an API/reference target for compatibility work.

## Project state

This branch should stay low-risk: cleanup/documentation only. Runtime feature work belongs in dedicated feature branches.

## Legal note

Wii2Xenon does not include Nintendo game assets, ROMs, ISOs, or proprietary SDK files. Development should use legally obtained software/data and open-source or independently written compatibility code.

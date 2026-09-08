# Wii2Xenon (Wii360)

Wii2Xenon is an experimental compatibility/porting layer for bringing Wii and GameCube software concepts and APIs to the Xbox 360.

## Branch

**`feature/gx360-texture-lod` — M0.4.1 GX texture filtering / LOD metadata**

This branch builds on the first successful textured-quad milestone and makes `GXTexObj` behave more like the texture objects expected by libogc clients.

## What this branch adds

- `GX_NEAR` and `GX_LINEAR`
- `GX_InitTexObjLOD()` compatibility surface
- storage for min/mag filters, min/max LOD, LOD bias, bias clamp, edge LOD and anisotropy metadata
- translation of `GX_NEAR` to point filtering on Xbox 360
- translation of `GX_LINEAR` to bilinear filtering on Xbox 360
- existing `GX_CLAMP`, `GX_REPEAT` and `GX_MIRROR` wrapping remains supported
- visual side-by-side filter regression test
- WiiXInput rumble remains active as an input regression check

```text
GXTexObj / GX_InitTexObjLOD
            |
      GX texture state
            |
          GX360
            |
 Xbox 360 sampler states
            |
          Xenos
```

## Test target

**Status: 🧪 ready for VS2010 / Xenia / later Xbox 360 validation**

Expected result:

- dark background
- two checkerboard quads side by side
- left quad uses `GX_NEAR` and should show crisp, hard-edged enlarged texels
- right quad uses `GX_LINEAR` and should look visibly smoother / blended
- holding A still activates the rumble regression test

The source texture is intentionally only 16x16 with a one-pixel checker pattern so the difference between point and bilinear filtering is easy to see.

## Current milestones

```text
M0.0   XEX build / boot                       ✅
M0.1   Direct3D Clear / Present               ✅
M0.2   WiiXInput digital input                ✅
M0.2.2 PAD analog / triggers                  ✅
M0.2.3 PAD / WPAD rumble                     ✅
M0.3.0 Standalone GX360 core                 ✅ Xenia + Xbox 360
M0.3.1 First GX-style triangle               ✅ Xenia + Xbox 360
M0.3.2 GX_QUADS + GX_TRIANGLESTRIP           ✅ Xenia / hardware pending
M0.3.x Interactive 3D triangle rotation      ✅ Xenia / hardware pending
M0.4.0 UVs + first textured GX quad          ✅ Xenia / hardware pending
M0.4.1 GX texture filtering / LOD metadata   🧪
```

## Scope note

`GX_InitTexObjLOD()` currently preserves the full metadata needed by the compatibility surface, but only the non-mip `GX_NEAR` and `GX_LINEAR` behavior is actively mapped to Xbox 360 sampler states. Real mip chains, LOD selection, LOD bias and anisotropy are intentionally deferred until a real client requires them.

The current `GX_TF_RGBA8` upload path still expects a simple linear 32-bit ARGB buffer created by our test code. Native Wii tiled/planar texture conversion is a later milestone.

## Why this matters for 240p Test Suite

The Wii 240p Test Suite uses `GX_InitTexObj()` and `GX_InitTexObjLOD()` when preparing textures, so implementing this compatibility surface now reduces the amount of client-specific rewriting we will need later.

## Next direction

After M0.4.1 is validated, the next visual milestone can introduce an external image path (PNG is a good development test), followed by native Wii texture/TPL conversion support required for real Wii assets.

## Legal note

Wii2Xenon does not include Nintendo game assets, ROMs, ISOs, or proprietary SDK files. Development should use legally obtained software/data and open-source or independently written compatibility code.

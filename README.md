# World of Zoo Performance Fix

**V4 is the current release, selected after local gameplay testing at 60 FPS.** This Windows fix for **World of Zoo (Steam, 32-bit)** improves specific Direct3D 9 buffer accesses, adds a 60 FPS cap, and corrects a fixed physics timestep used by the previous 60 FPS build.

[Download V4](https://github.com/DeadneM/World-of-Zoo-Performance-Fix/releases/tag/v4) · [Instructions en français](README.fr.md) · [Technical notes](docs/TECHNICAL.md) · [V2 / native frame cap](https://github.com/DeadneM/World-of-Zoo-Performance-Fix/releases/tag/v2)

## Install

1. Close the game and extract **World-of-Zoo-Performance-Fix-v4.zip**.
2. Open the game folder containing `WoZRetail.exe` (Steam → Properties → Installed Files → Browse).
3. Back up any existing `d3d9.dll` outside the game folder.
4. Copy **`d3d9.dll` and `WoZPerformanceFix.ini`** from the archive root beside `WoZRetail.exe`.
5. Launch normally through Steam.

If V3 or the V4 test build is already installed with `TargetFPS=60` and `ReadOnlyMeshQueries=1`, the existing INI is compatible. This release preserves the exact V4 DLL used in those tests.

## What changes

- **Menu buffers:** retains V2's targeted DISCARD correction for complete dynamic UI index uploads.
- **CPU-accessed meshes:** retains V2's readable managed vertex/index buffers, preserving data needed for CPU queries and updates.
- **Triangle queries:** uses READONLY only on the verified read path, retaining normal locks for writes.
- **60 FPS:** replaces the native 33 ms wait block with QPC-based pacing and a Windows high-resolution waitable timer when available.
- **Physics timestep:** passes the measured frame duration to the identified world-step path, replacing its fixed 1/30-second duration. The pacing and physics patches are installed together or both refused.

The fix applies in process memory. `WoZRetail.exe` on disk is unchanged, and Direct3D objects/vtables remain those supplied by Windows.

## Settings and rollback

The supplied INI selects **60 FPS**. The gameplay evaluation for this release covers 60 FPS; higher targets are not validated.

To retain the native frame limiter and physics step, set `TargetFPS=0`, then restart the game. This means about 30 FPS, not unlimited FPS. `ReadOnlyMeshQueries=0` also restores V2's lock behavior.

For the exact published V2, close the game and copy `Retour-V2/d3d9.dll` from the archive over the V4 DLL. V2 ignores the V4 INI. To remove the fix entirely, remove the added DLL and restore any earlier wrapper you backed up.

## Validation and scope

Local playtesting showed a mostly 59–60 FPS overlay with V4, compared with approximately 30 FPS in the V2 reference capture. The maintainer accepted V4 for publication after those comparisons on 22 September 2026. Results depend on hardware, drivers and scene complexity.

Native-instruction tests reproduce the old world-step path receiving 20 seconds of total duration during 600 updates spread across 10 real seconds. The corrected path receives 10 seconds. The integration endpoint is mocked in that test; it does not simulate the entire game. Buffer, pacing, ABI, initialization-failure and Windows Direct3D-forwarding tests pass.

This is not a guarantee that every animation or counter is independent of frame rate. The reference captures contain different actions, so they do not establish an exact speed ratio for every animation.

## Log and compatibility

Read the newest session in `WoZPerformanceFix.log`. It should include `APPLIED V4: target 60 FPS` and `Physics step uses actual frame delta`. The preserved test binary still prints **V4 experimental** in its banner; this identifies the same accepted DLL, not another build.

Nineteen code signatures were checked against the analyzed Steam image. The original supported `WoZRetail.exe` SHA-256 is:

```text
622cd4914c4f85f8af949100746078be6a6c111303758cc74206f210e813bfc3
```

The V4 `d3d9.dll` is x86 PE32, 218624 bytes, SHA-256:

```text
171d0c13914635db7f3d16088399ad22888090a5e0d340c0738c9ef8d4558fc5
```

See [BUILDING.md](BUILDING.md) for sources and test instructions. No game executable, game assets, saves, private captures or raw process logs are distributed. When reporting an issue, include the fix version, game checksum, graphics driver, scene and relevant log lines.

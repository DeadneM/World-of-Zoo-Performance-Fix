# Technical notes — V4

Addresses refer to the analyzed Steam x86 image at base `0x00400000`. The proxy uses module-relative patch offsets and verifies expected instruction bytes. V2's original buffer investigation is preserved in [TECHNICAL-V2.md](TECHNICAL-V2.md).

## Inherited buffer changes

The UI full-index upload at `0x7087E0` uses DISCARD only for the verified dynamic-buffer caller returning to `0x62C91B`. The mesh factory at `0x833470` selects the existing readable managed-buffer branch; retained contents remain available to deformation and CPU triangle queries.

The paired lock at `0x70FE80` additionally selects engine mode 3 (READONLY) only when both return-address guards (`0x8330AB`, `0x83352A`) and readable managed VB/IB properties match. A copy of the original paired-lock routine supplies the normal path, and a second copy changes the two mode arguments for that read-only path. Updates and cleanup remain native. There are no COM object/vtable replacements.

The earlier linear scan covered all executable sections but does not establish an exhaustive dynamic call graph. Buffers requiring retained contents must not receive a global DISCARD conversion. Existing DISCARD/NOOVERWRITE vertex-ring behavior remains in place.

## Frame pacing

The native main-loop block at `0x40DA40–0x40DA6C` waits until 33 ms have elapsed. Optional pacing replaces its first seven bytes, preserving CPU registers, flags and x87/MMX/SSE state in the detour. QPC deadlines include work between frames; late frames reset the schedule without catch-up bursts. The implementation uses a high-resolution waitable timer when available, with a short final active wait. It preserves the native enable/inactive state checks.

The supplied target and the gameplay validation scope are 60 FPS. The inherited parser accepts integer targets from 30 to 360, but higher values have no gameplay validation. `TargetFPS=0` keeps the native limiter **and** native physics timestep. Neither the game graphics settings nor VSync are altered by this DLL.

## Physics duration

The native clock at `0x4EFE90` measures elapsed milliseconds, applies its time scale and clamps large frame deltas to 100 ms. This clock was already correct in V3.

The controller constructor at `0x7DF9F0` sets `[controller+0x20]` to 1/30 second. Its update at `0x7DF7F0` reads the clock delta, updates an accumulator and invokes the world-step path for a positive delta. The step at `0x7DF840` still loads `+0x20` before passing a float duration to `0x7FB060`.

V4 detours seven bytes at `0x7DF807` (RVA `0x3DF807`). A 15-byte stub inserts `FST dword [ESI+0x20]`, leaves ST(0) intact, reproduces the overwritten instructions and returns to `0x7DF80E`. The original step and its timestep getter at `0x7DF950` then read the measured frame duration. Zero-delta skipping, event dispatch and the existing clock clamp remain native.

The limiter and physics detours form one installation transaction: validate both paths, prepare both stubs, then acquire write access to both sites before changing either. A refused second protection change restores the first protection and leaves both sites unmodified.

This changes one timestep source. Counters adding a fixed amount per frame are not automatically corrected, and not every animation consumer has been traced.

## Evidence and limits

The physics regression executes the original clock, accumulator, controller-update, step and getter instructions with mocked event endpoints and a mocked world-integration endpoint that adds received durations. With 600 updates distributed over 10 seconds, the original path supplies approximately 20 seconds and V4 supplies 10 seconds. Tests at 20, 30, 40, 50 and 60 updates/sec, plus 10,000 variable-duration updates, pass. The test verifies the call duration, not the full Havok world.

The V2 buffer regressions, guarded READONLY path, pacing/ABI tests and system Direct3D forwarding pass. Nine full-initialization scenarios include mismatched physics/update signatures and refusal to make the second site writable. Nineteen instruction signatures match the analyzed image; the DLL exports only `Direct3DCreate9`.

Local captures showed mostly 59–60 FPS with V4 and approximately 30 FPS with V2. The maintainer accepted this exact V4 binary for publication on 22 September 2026. The recordings contain different actions and are not an objective frame-time benchmark or a matched timing test of every animation. Hardware/driver and scene coverage remain limited to the reported local tests.

The packaged reference DLL retains its historical `V4 experimental` log banner. Its SHA-256 is `171d0c13914635db7f3d16088399ad22888090a5e0d340c0738c9ef8d4558fc5` (218624 bytes). The release does not silently rebuild that accepted binary.

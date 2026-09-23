# Changelog

## Repository maintenance — 2026-09-23

- Rewrite the English and French guides from installation through troubleshooting, validation and implementation details.
- Keep downloadable ZIPs in Releases instead of duplicating them in the source tree.
- Consolidate the duplicate UI/mesh tests into `test_buffers.c`, retaining the optional native-constructor check.
- Require explicit analysis-image paths in native tests and reject unknown initialization cases.
- Give the active buffer/pacing and physics headers descriptive names; move the standalone pacing installer into its ABI test.
- Preserve the V4 runtime logic, log strings and exact published binary. Published tags and release archives retain their original snapshots.

## V4 — 2026-09-23 (accepted 2026-09-22)

- Publish the exact V4 DLL accepted after local gameplay comparisons, targeting 60 FPS.
- Retain V2's menu/mesh buffer fixes and V3's guarded READONLY triangle queries and QPC pacing.
- Replace the identified fixed physics timestep with the native measured frame duration.
- Install the pacing and physics patches together, with native fallback if either cannot be installed.
- Include the default INI, sources, tests, checksums and the exact V2 DLL for rollback.

The V4 DLL SHA-256 is `171d0c13914635db7f3d16088399ad22888090a5e0d340c0738c9ef8d4558fc5`. Its historical experimental log banner is retained to preserve the tested binary. Local acceptance does not claim exhaustive animation or hardware validation.

## V3 — internal test build

- Add guarded READONLY CPU triangle queries and configurable QPC pacing.
- Local tests report smooth 60 FPS, with a concern about accelerated gameplay.
- The native clock was correct, but the fixed physics-step path motivated V4.

## V2 — native frame-cap reference

- Preserve the targeted V1 menu index-buffer correction.
- Use the engine's existing managed allocation branch for the observed CPU-accessed mesh factory.
- Make managed vertex/index buffers in that paired-mesh class readable, retaining their contents for CPU queries and updates.
- Keep the native 33 ms limiter and original Windows Direct3D objects.
- Validate nine code signatures before patching.

The reference DLL has SHA-256 `9bb09d0ca77b6259d8785323bdadfe556d925602837ec4a7439bd9e1c9eaf565`. This repository preserves that exact tested file.

## V1

- Apply DISCARD only to the dynamic UI index upload identified in the menu sample.
- Menu improvement reported during local testing; gameplay stalls remained and motivated V2.

<div align="center">

# World of Zoo Performance Fix

<a href="https://steamcommunity.com/sharedfiles/filedetails/?id=3806665895">
  <img src="docs/images/Image%20Codex%2023%20sept.%202026%2C%2012_21_15.png" alt="World of Zoo Performance Fix installation guide">
</a>

**Smoother menus, fewer buffer stalls, and a 60 FPS mode with corrected physics timing.**

World of Zoo · Steam · Windows · Direct3D 9 · 32-bit

**[Download V4](https://github.com/DeadneM/World-of-Zoo-Performance-Fix/releases/tag/v4)** · **[Lire en français](README.fr.md)** · [Report an issue](https://github.com/DeadneM/World-of-Zoo-Performance-Fix/issues)

</div>

V4 is the current release, accepted after local gameplay comparisons at **60 FPS**. It fixes specific graphics-buffer accesses and replaces a fixed physics timestep with the measured duration of each frame. Installation takes two files; the original game executable stays intact.

> **Just want to play?** Download `World-of-Zoo-Performance-Fix-v4.zip`, then follow the installation below. The included settings already select 60 FPS.

## Contents

- **Play:** [Install](#install) · [Check it works](#check-it-works) · [Settings](#settings)
- **Get help:** [Compatibility](#compatibility) · [Troubleshooting](#troubleshooting) · [Rollback and uninstall](#rollback-and-uninstall)
- **Understand:** [What the fix changes](#what-the-fix-changes) · [Validation](#validation) · [Technical details](#technical-details) · [Source and development](#source-and-development)

## Install

1. **Close the game.** Download and extract **[World-of-Zoo-Performance-Fix-v4.zip](https://github.com/DeadneM/World-of-Zoo-Performance-Fix/releases/download/v4/World-of-Zoo-Performance-Fix-v4.zip)** from the release's **Assets** section. GitHub's automatic “Source code” downloads do not contain the ready-to-use DLL.
2. **Find the game folder.** In Steam: right-click World of Zoo → **Properties → Installed Files → Browse**. Locate `WoZRetail.exe`.
3. **Back up any existing `d3d9.dll`** outside that folder. Another mod or graphics wrapper may already use this name.
4. Open the extracted **`World-of-Zoo-Performance-Fix-v4`** folder and **copy these two files** beside `WoZRetail.exe`:

   ```text
   World of Zoo/
   ├── WoZRetail.exe
   ├── GameLauncher.exe
   ├── d3d9.dll                 ← the fix
   └── WoZPerformanceFix.ini    ← the settings
   ```

5. **Launch normally through Steam.** No installer or manual executable patch is needed.

The source, documentation and `Retour-V2` folder are optional for playing. Keep them outside the game folder. An existing V3/V4 INI is compatible when it uses `TargetFPS=60` and `ReadOnlyMeshQueries=1`.

## Check it works

After launching, open **`WoZPerformanceFix.log`** beside the DLL and read the **last session** at the bottom. With the default settings, look for:

```text
APPLIED V4: target 60 FPS
Physics step uses actual frame delta
Original Windows Direct3DCreate9: OK
```

The first two are excerpts from the same log line. `APPLIED V2` and `APPLIED V3` lines are also normal: V4 includes those earlier buffer fixes.

The released DLL still prints **`V4 experimental`** and **`Gameplay validation required`**. These historical messages were retained to publish exactly the binary accepted during playtesting. They do not identify a different download.

## Settings

Edit **`WoZPerformanceFix.ini`** beside the DLL, save it, then **restart the game**.

```ini
[Timing]
TargetFPS=60

[Buffers]
ReadOnlyMeshQueries=1
```

| Setting | Value | Meaning |
| --- | --- | --- |
| `TargetFPS` | `60` (default) | Enables the new frame limiter and the corresponding physics-timing correction. **60 FPS is the gameplay-tested target.** |
| `TargetFPS` | `0` | Keeps the game's native limiter and native physics step, around 30 FPS. **Zero does not mean unlimited.** |
| `ReadOnlyMeshQueries` | `1` (default) | Enables guarded read-only access for the identified CPU triangle-query path. |
| `ReadOnlyMeshQueries` | `0` | Restores V2's normal mesh-lock behavior; the other buffer fixes remain active. |

For the native timing and V2-style locks within V4, use `TargetFPS=0` and `ReadOnlyMeshQueries=0`. For the exact V2 binary, see [rollback](#rollback-and-uninstall).

**Above 60 FPS:** the parser accepts integer targets from 30 to 360, but gameplay at those higher targets has not been validated. Use 60 for this release. Unsupported numeric targets fall back to 60. The DLL does not change VSync, resolution or graphics settings; other limits can still affect the achieved frame rate.

## Compatibility

| Component | Supported scope |
| --- | --- |
| Game | The analyzed **Steam Windows version** of World of Zoo, `WoZRetail.exe` |
| Architecture | **x86 / 32-bit**, including when the game runs on 64-bit Windows |
| Graphics | Windows **Direct3D 9** |
| Other editions | Not validated; expected instruction signatures must match |
| Other wrappers | Automatic chaining with another `d3d9.dll` is not implemented |

The fix forwards `Direct3DCreate9` to the Windows system library. It does not chain-load ReShade, DXVK, dgVoodoo or another local proxy. Keep a backup of any existing wrapper before replacing it. No game executable, game assets or saves are included.

## Troubleshooting

| Symptom | What to check |
| --- | --- |
| No log appears | Confirm that the actual file is named `d3d9.dll`, that it is next to `WoZRetail.exe`, and that the game can write in that folder. Check for an accidentally nested extraction folder. |
| Still around 30 FPS | Check `TargetFPS=60`, restart, and look for the V4 success line in the latest log session. Also check VSync or an external frame cap. |
| `NOT APPLIED: executable signatures do not match` | The running image does not match the expected code. Compare the original executable's checksum below and check for another mod that changes those instructions. |
| `FPS/PHYSICS MODE NOT APPLIED` | The paired timing patch was refused. Native timing remains active; other successfully installed buffer fixes may still be active. Include this log line in an issue. |
| `READONLY NOT APPLIED` | The optional read-only path was refused. See the other log lines to check which buffer fixes were installed. |
| Stutters, unusual speed or a crash | Compare the same scene with `TargetFPS=0`, then with the supplied V2 DLL if needed. Report the scene, settings and relevant log lines. |
| The log mentions V2, V3 or “experimental” | This is expected for the exact V4 reference DLL; see [Check it works](#check-it-works). |

When [reporting a problem](https://github.com/DeadneM/World-of-Zoo-Performance-Fix/issues), include the fix version, Windows version, GPU and driver, game-executable checksum, INI settings, scene and steps to reproduce. Mention whether V2 behaves differently. Relevant log lines are enough; do not upload the game executable or a memory dump.

## Rollback and uninstall

**Return to V2:** close the game, then replace the installed DLL with **`Retour-V2/d3d9.dll`** from the V4 archive. This is the exact published V2 binary. V2 ignores the V4 INI. The [separate V2 release](https://github.com/DeadneM/World-of-Zoo-Performance-Fix/releases/tag/v2) remains available.

**Remove the fix:** close the game and remove the `d3d9.dll` supplied by this project. You can also remove its `WoZPerformanceFix.ini` and `WoZPerformanceFix.log`. Restore any earlier `d3d9.dll` you backed up. The game executable needs no restoration because this fix only patches memory while the game is running.

## What the fix changes

| Area | Problem addressed | V4 behavior |
| --- | --- | --- |
| Menus | A full UI index-buffer replacement used an ordinary lock that could wait for GPU work. | Uses DISCARD for the verified dynamic UI upload. |
| Animated meshes and CPU queries | The CPU needs to read and preserve mesh contents, making some GPU-backed accesses expensive. | Uses the engine's readable managed-buffer path for the identified mesh class. |
| Triangle queries | A verified read path used ordinary read/write locks. | Requests READONLY only when both caller guards and buffer properties match. |
| Frame limit | The main loop waited for roughly 33 ms per iteration. | Uses precise pacing with a 60 FPS target by default. |
| Physics timing | The identified world-step path still received 1/30 second for every update at 60 FPS. | Passes the measured frame duration to that path. |

These are targeted changes. Converting every buffer lock to DISCARD would lose contents that some game routines need. Raising FPS alone also does not correct every counter that advances by a fixed amount per frame.

## Validation

| Evidence | Result and scope |
| --- | --- |
| Local gameplay captures | V4 mostly **59–60 FPS**, compared with approximately **30 FPS** in V2. Accepted for release after that comparison. |
| Native physics-instruction regression | At 600 updates over 10 real seconds, the old path passes about **20 seconds** to the world-step endpoint; the corrected path passes **10 seconds**. |
| Automated checks | Buffer guards, native instructions, pacing, register/state preservation, failure paths and Windows Direct3D forwarding pass. |
| Full initialization | Nine cases cover default/native settings, invalid targets, signature mismatches and refusal of the second timing-patch write. |

The physics test uses a mocked world-integration endpoint; it does not simulate a full game. The two gameplay recordings contain different actions, so they do not establish exact timing for every animation. Results remain dependent on hardware, drivers and scene complexity. The optional graphics-device benchmark did not run successfully in the original restricted test environment; no GPU benchmark result is claimed from it.

## Technical details

V4 is a small Direct3D 9 proxy that applies checked, module-relative x86 patches when the game first calls `Direct3DCreate9`. It preserves the original Windows Direct3D objects and vtables. The timing patches are installed together: both are validated and made writable before either code site changes.

- **Buffers:** guarded UI DISCARD, readable MANAGED meshes, and guarded READONLY triangle queries.
- **Pacing:** `QueryPerformanceCounter` deadlines, a high-resolution waitable timer when available, and a short final wait. Late frames reset the schedule without catch-up bursts. No global timer-resolution change is requested.
- **Physics:** store the native measured delta in `[controller+0x20]` before the existing world-step path uses it. The elapsed-time clock and its 100 ms clamp stay in place.
- **Instruction checks:** 19 signatures were verified for the analyzed image. Runtime checks gate the corresponding patch groups; the DLL does not calculate the whole executable's SHA-256 at launch.

Read [the implementation notes](docs/TECHNICAL.md) for addresses, signatures and timing behavior, or [the buffer investigation](docs/TECHNICAL-V2.md) for the original evidence behind the V2 changes.

<details>
<summary><strong>Reference files and SHA-256 checksums</strong></summary>

These identify the analyzed game and the exact published files. Rebuilt DLLs may have different metadata and checksums.

| File | Bytes |
| --- | ---: |
| Original supported `WoZRetail.exe` | 7,405,568 |
| V4 `d3d9.dll` | 218,624 |
| V4 release ZIP | 248,142 |
| V2 rollback `d3d9.dll` | 213,504 |

```text
WoZRetail.exe — original supported Steam executable
622cd4914c4f85f8af949100746078be6a6c111303758cc74206f210e813bfc3

d3d9.dll — V4
171d0c13914635db7f3d16088399ad22888090a5e0d340c0738c9ef8d4558fc5

World-of-Zoo-Performance-Fix-v4.zip
b843a98304021771914cb1d07bf6a763f6722380d07addf2a10bb2a5af62a9f5

Retour-V2/d3d9.dll
9bb09d0ca77b6259d8785323bdadfe556d925602837ec4a7439bd9e1c9eaf565
```

From the relevant folder, PowerShell can check a file with:

```powershell
Get-FileHash -Algorithm SHA256 .\d3d9.dll
```

The [release checksum file](https://github.com/DeadneM/World-of-Zoo-Performance-Fix/releases/download/v4/SHA256SUMS.txt) identifies the ZIP. The archive also contains checksums for its contents.

</details>

## Source and development

| Path | Purpose |
| --- | --- |
| [`src/woz_fix.c`](src/woz_fix.c) | Proxy initialization, configuration, image checks and inherited buffer patches |
| [`src/woz_buffer_pacing.h`](src/woz_buffer_pacing.h) | Guarded mesh queries and pacing helpers |
| [`src/woz_physics.h`](src/woz_physics.h) | Physics timestep and paired timing-patch installation |
| [`src/woz_fix.def`](src/woz_fix.def) | The `Direct3DCreate9` export |
| [`tests/`](tests/) | Buffer, physics, timing, initialization and forwarding tests |
| [`BUILDING.md`](BUILDING.md) | Windows x86 build commands, test instructions and analysis-image requirements |
| [`CHANGELOG.md`](CHANGELOG.md) · [`version.json`](version.json) | Version history and machine-readable release metadata |

Ready-to-use binaries live in **[Releases](https://github.com/DeadneM/World-of-Zoo-Performance-Fix/releases)**. The main branch contains the maintained source and documentation; tags and release archives preserve their published snapshots. Development cleanup does not replace the accepted V4 DLL.

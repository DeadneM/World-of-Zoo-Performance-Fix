# World of Zoo Performance Fix

A small Windows compatibility fix for **World of Zoo (Steam, 32-bit)**. It targets Direct3D 9 buffer stalls in menus and CPU-accessed 3D meshes while preserving the game's native **33 ms frame limiter (about 30 FPS)**.

**Current reference build: V2.** This is the exact build selected after local testing, with the V1 menu fix included. It is a compatibility fix, not an FPS unlocker. Results can vary between graphics drivers and scenes; no universal FPS improvement is claimed.

[Instructions en français](README.fr.md) · [Download V2](dist/World-of-Zoo-Performance-Fix-v2.zip) · [Technical notes](docs/TECHNICAL.md)

## Install

1. Close the game and extract the V2 ZIP.
2. Open the game installation folder containing `WoZRetail.exe` (Steam → Properties → Installed Files → Browse).
3. If a `d3d9.dll` is already present, keep a backup outside the game folder. For the previous V1 fix, you can rename it to `d3d9-WoZIndexFix-V1.disabled` instead.
4. Copy **only `d3d9.dll`** from the ZIP into the game folder.
5. Launch normally through Steam.

Only one local `d3d9.dll` can occupy this location. Keep any existing wrapper or mod backed up before replacing it.

The fix writes `WoZIndexFix.log` next to the game. The latest session should contain:

```text
WoZ Index and Mesh Fix V2
APPLIED: dynamic UI index uploads use DISCARD.
APPLIED: CPU-accessed meshes use readable MANAGED vertex/index buffers.
Original Windows Direct3DCreate9: OK
```

The actual `APPLIED` lines include additional details. `NOT APPLIED` means the executable signatures do not match or the memory patch could not be installed.

## Remove or return to V1

Close the game and remove the added `d3d9.dll`. Restore your previous DLL if you had one. To return to V1, restore its backup as `d3d9.dll`.

The game executable on disk, save files, simulation speed and frame limiter are not modified. The corrections apply to the process memory when Direct3D initializes.

## What V2 changes

- **Menus:** the dynamic UI index buffer is replaced using `D3DLOCK_DISCARD` on the specific upload path. Other callers and static buffers retain the original behavior.
- **3D meshes:** the observed CPU-accessed mesh factory uses the game's existing managed-buffer path. Managed vertex/index buffers in that paired-mesh class are readable, allowing retained mesh data to stay available to CPU updates and triangle queries. Other callers requesting dynamic buffers retain that branch.
- The proxy forwards `Direct3DCreate9` to Windows. It does not replace Direct3D objects or their vtables.

Nine code signatures are checked before patching. Only the following original Steam executable has been analyzed:

```text
WoZRetail.exe SHA-256
622cd4914c4f85f8af949100746078be6a6c111303758cc74206f210e813bfc3
```

The repository contains the fix and its source, not the game executable or assets.

## V2 binary checksum

```text
d3d9.dll (213504 bytes)
9bb09d0ca77b6259d8785323bdadfe556d925602837ec4a7439bd9e1c9eaf565
```

This checksum identifies the preserved reference DLL. A local rebuild may have a different checksum.

## Build and tests

See [BUILDING.md](BUILDING.md). The reference build was compiled with Zig 0.15.2 for `x86-windows-gnu`. Tests exercise the actual x86 detour and the original mesh constructor using mock engine objects, including 10,000 repetitions for each path.

The Windows Direct3D forwarding test passed. A separate graphics-device test could not run in the original restricted environment (`D3DERR_INVALIDCALL` during device creation); it is provided as an optional test, not as evidence of a measured performance gain. The installed V2 log confirmed both patches were applied successfully.

When reporting an issue, include the game executable checksum, graphics driver version, affected scene, and relevant `WoZIndexFix.log` lines. Check attachments for personal paths before sharing them.

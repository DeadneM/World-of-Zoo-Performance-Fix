# Building and testing

[User guide](README.md) · [Guide en français](README.fr.md) · [Technical notes](docs/TECHNICAL.md)

These instructions target the current main branch on **Windows with Zig 0.15.2**. The game, DLL and test executables are **32-bit**, including on 64-bit Windows. Install Zig from [ziglang.org](https://ziglang.org/download/) and make `zig` available in your terminal.

The published V4 DLL is a preserved reference binary. A local rebuild can differ in metadata/checksum; do not substitute it into the existing release without new validation.

## Build the DLL

Open PowerShell in the repository root:

```powershell
New-Item -ItemType Directory -Force build | Out-Null
zig cc -target x86-windows-gnu -O2 -Wall -Wextra -Werror -Wno-unused-parameter -shared src/woz_fix.c src/woz_fix.def -o build/d3d9.dll
```

Keep both headers beside `woz_fix.c`: `woz_buffer_pacing.h` and `woz_physics.h`. The DLL exports only `Direct3DCreate9`. Build products belong in the ignored `build/` directory.

## Build the core tests

```powershell
$testFlags = @('-target', 'x86-windows-gnu', '-O2', '-Wall', '-Wextra', '-Werror', '-Wno-unused-parameter', '-Wno-unused-function')
$testNames = @('test_buffers', 'test_physics', 'test_performance', 'test_initialization', 'test_forward')
foreach ($testName in $testNames) {
    & zig cc @testFlags "tests/$testName.c" -o "build/$testName.exe"
    if ($LASTEXITCODE -ne 0) { throw "Build failed: $testName" }
}
```

Tests explicitly undefine `NDEBUG` before including `assert.h`: Zig defines it at `-O2`, and assertions must remain active.

## Run without a game-analysis image

```powershell
& .\build\test_buffers.exe
& .\build\test_forward.exe (Resolve-Path .\build\d3d9.dll).Path
```

The buffer test runs the self-contained UI regression and explicitly skips the native mesh-constructor portion when no analysis image is supplied. The forwarding test uses the Windows Direct3D library and verifies that the returned object/vtable comes from the system module.

## Run the native-instruction tests

These tests execute copied x86 instructions from a **local analysis copy** of the supported game. That file is not distributed. The tests depend on its RVA/raw layout and verified instruction ranges; an arbitrary original executable is not a substitute. Never install the analysis copy over the game's original executable.

Expected analysis-copy SHA-256:

```text
136d27d7fd929e27da5d931a7b0e0df751673e070e99965bf2ef88b6f4826390
```

Set the path to your matching analysis copy, then run from the repository root:

```powershell
$analysisImage = 'C:\path\to\WoZRetail.exe.unpacked.exe'
$expectedHash = '136d27d7fd929e27da5d931a7b0e0df751673e070e99965bf2ef88b6f4826390'
if ((Get-FileHash -Algorithm SHA256 -LiteralPath $analysisImage).Hash -ne $expectedHash) {
    throw 'The analysis image does not match the verified fixture.'
}

foreach ($testName in @('test_buffers', 'test_physics', 'test_performance')) {
    & ".\build\$testName.exe" $analysisImage
    if ($LASTEXITCODE -ne 0) { throw "Test failed: $testName" }
}

$initCases = @('on', 'native', 'invalid', 'mismatch', 'fps_mismatch', 'query_mismatch', 'physics_mismatch', 'step_mismatch', 'protect_failure')
foreach ($initCase in $initCases) {
    & .\build\test_initialization.exe $initCase $analysisImage
    if ($LASTEXITCODE -ne 0) { throw "Initialization test failed: $initCase" }
}
```

Initialization tests overwrite an INI and append a log **beside their test executable**. Keep and run them in `build/`, never in the game directory. They use an in-memory fixture; the input analysis file is opened read-only.

## What each test covers

| Test | Coverage |
| --- | --- |
| `test_buffers.c` | UI caller/dynamic guards and actual detour execution; with an analysis image, the native mesh constructor, retained contents/flags and 10,000 constructions. Consolidates the former `test_fix.c` and `test_v2_regression.c`. |
| `test_physics.c` | Native clock, accumulator, update, step and getter instructions; fixed versus measured timestep; 20–60 updates/sec and 10,000 variable frames; zero delta, pause and 100 ms clamp. World integration is mocked. |
| `test_performance.c` | Native clock and paired-lock paths, READONLY guards, failure cleanup, 10,000 pacing-stub ABI calls and synthetic CPU timing. Its standalone pacing installer is test-only. |
| `test_initialization.c` | Nine runtime configuration and patch-refusal cases, including refusal to make the second timing site writable. Requires both a known case name and an explicit analysis-image path. |
| `test_forward.c` | System Direct3D object/vtable forwarding from an unsupported host image. |

CPU timer measurements are not gameplay FPS results. Mocked integration does not validate every animation or scene.

## Optional graphics-device benchmark

```powershell
zig cc -target x86-windows-gnu -O2 -Wall -Wextra -Werror -Wno-unused-parameter tests/test_managed_mesh.c -luser32 -o build/test_managed_mesh.exe
& .\build\test_managed_mesh.exe
```

This requires a working Direct3D 9 graphics device. It compares a synthetic dynamic/default-buffer workload with readable managed buffers and checks retained data after rendering and updates. Device creation failed in the original restricted environment (`0x8876086C`), so this test supplies no release performance claim.

## Published snapshots

The `v4` tag and downloadable archive retain their original source layout: `woz_experimental_v3.h`, `woz_physics_v4.h` and the earlier test names. When building that archive, enter its `Sources` directory and follow the `BUILDING.md` shipped with it. The main-branch cleanup renames helpers and consolidates tests without changing the production C token stream or replacing the released DLL.

Release ZIPs and their checksum files are maintained in [GitHub Releases](https://github.com/DeadneM/World-of-Zoo-Performance-Fix/releases), rather than duplicated in the source tree.

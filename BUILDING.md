# Building and testing V4

Use Windows and Zig 0.15.2. The game, DLL and tests are **32-bit**, including on 64-bit Windows.

From the repository root, create a `build` directory, then run the command below. Inside the downloadable archive, enter the `Sources` directory first.

```text
zig cc -target x86-windows-gnu -O2 -Wall -Wextra -Werror -Wno-unused-parameter -shared src/woz_fix.c src/woz_fix.def -o build/d3d9.dll
```

Keep `woz_experimental_v3.h` and `woz_physics_v4.h` beside `woz_fix.c`. V4 reuses the V3 buffer/pacing code. The release archive preserves the exact locally tested reference DLL; a rebuild can have different metadata/checksum.

## Tests

Compile each test with the same compiler flags, adding `-Wno-unused-function`, omitting `-shared` and the .def file. For example:

```text
zig cc -target x86-windows-gnu -O2 -Wall -Wextra -Werror -Wno-unused-parameter -Wno-unused-function tests/test_physics.c -o build/test_physics.exe
build/test_physics.exe "path/to/WoZRetail.exe.unpacked.exe"
```

Tests with assertions explicitly undefine NDEBUG before including assert.h, because Zig defines it at -O2.

| Test | Arguments and purpose |
| --- | --- |
| `test_physics.c` | `<analysis-exe>`: executes native clock, accumulator, physics-update, step and timestep-getter instructions; reproduces the old doubled duration at 60 updates/sec and verifies corrected time at 20–60 updates/sec and 10,000 variable frames. The world-integration endpoint is a mock. |
| `test_performance.c` | `<analysis-exe>`: native clock and paired-lock paths, guards, failure cleanup, 10,000 detour ABI calls and synthetic CPU pacing. |
| `test_v2_regression.c` | `<analysis-exe>`: inherited UI and managed-mesh paths. |
| `test_initialization.c` | `<case> <analysis-exe>`: runtime configuration and signature checks. Cases: `on`, `native`, `invalid`, `mismatch`, `fps_mismatch`, `query_mismatch`, `physics_mismatch`, `step_mismatch`, `protect_failure`. |
| `test_forward.c` | `<absolute DLL path>`: original Windows Direct3D object/vtable forwarding; unsupported host image stays unpatched. |
| `test_fix.c` | Legacy self-contained UI test; optional `<analysis-exe>` enables its native-constructor checks. |

Initialization tests write an INI and log alongside the test executable. Run them in a test directory, never the game directory. CPU timer measurements are not gameplay FPS measurements.

The locally supplied unpacked analysis copy has SHA-256:

```text
136d27d7fd929e27da5d931a7b0e0df751673e070e99965bf2ef88b6f4826390
```

It is not distributed. Tests depend on its verified instruction ranges and RVA/raw layout. Never replace the installed original game executable with this analysis copy.

## Optional graphics-device test

`test_managed_mesh.c` can be built with `-luser32`. It needs a working Direct3D 9 device and measures a synthetic buffer workload. Device creation failed in the original restricted development environment, so it supplies no release performance claim.

# Building and testing

Use Windows and [Zig 0.15.2](https://ziglang.org/download/). The game and proxy are **32-bit**, including on 64-bit Windows.

From the repository root:

```bat
zig cc -target x86-windows-gnu -O2 -Wall -Wextra -Werror -Wno-unused-parameter -shared src/woz_fix.c src/woz_fix.def -o build/d3d9.dll
```

Create `build` first. This is the same compiler version and build configuration as the reference DLL; debug metadata and other build-environment details may produce a different checksum. The unchanged reference binary is preserved in `dist/World-of-Zoo-Performance-Fix-v2.zip`.

## Detour and constructor tests

```bat
zig cc -target x86-windows-gnu -O2 -Wall -Wextra -Werror -Wno-unused-parameter tests/test_fix.c -o build/test_fix.exe
build\test_fix.exe
```

Without arguments, the self-contained UI-detour tests run and the original-constructor test is explicitly skipped. To run the latter too, supply the path to your own readable analysis copy of the supported game executable:

```bat
build\test_fix.exe "path\to\WoZRetail.exe.unpacked.exe"
```

The analysis copy used for the regression test has SHA-256:

```text
136d27d7fd929e27da5d931a7b0e0df751673e070e99965bf2ef88b6f4826390
```

It is not included. The production fix is installed with the original game executable, not an analysis copy.

Assertions are explicitly enabled in the test even though Zig defines `NDEBUG` at `-O2`.

## Direct3D forwarding

```bat
zig cc -target x86-windows-gnu -O2 -Wall -Wextra -Werror tests/test_forward.c -o build/test_forward.exe
build\test_forward.exe "build\d3d9.dll"
```

This verifies the original Windows object/vtable and graceful refusal to patch an unsupported executable. It does not require starting the game.

## Optional graphics test

```bat
zig cc -target x86-windows-gnu -O2 -Wall -Wextra -Werror tests/test_managed_mesh.c -luser32 -o build/test_managed_mesh.exe
build\test_managed_mesh.exe
```

This requires access to a working Direct3D 9 hardware device. It compares the original buffer allocation pattern and readable managed buffers in a small synthetic workload. It is not a World of Zoo FPS benchmark. Device creation failed in the original restricted development environment, so no timing result from this test was used to validate the release.

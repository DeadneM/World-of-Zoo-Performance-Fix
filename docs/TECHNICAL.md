# Technical notes — V2

Addresses below refer to the analyzed 32-bit Steam image at base `0x00400000`. The packaged DLL uses module-relative offsets and verifies expected code bytes before patching.

## UI index uploads

The UI full-index upload returns to `0x0062C91B` and reaches the engine upload method at `0x007087E0`. The buffer is created with `D3DUSAGE_DYNAMIC | D3DUSAGE_WRITEONLY`, but the original upload locks offset zero with engine mode 0.

V1 replaces six bytes at RVA `0x3087E6` with a guarded detour. The specific UI caller and the buffer's dynamic flag must both match. Only then is engine mode 1 selected, which the original lock method translates to `D3DLOCK_DISCARD`. The original copy, unlock and bind remain in place.

A pre-fix menu sample found 279 of 300 observations in a Windows wait, with the index-buffer lock chain present. This observation and the caller's full replacement semantics supported the targeted change. The menu improvement was then reported during testing.

Microsoft describes why unflagged locks on buffers still in use can stall, and when DISCARD is appropriate: [D3D9 performance optimizations](https://learn.microsoft.com/en-us/windows/win32/direct3d9/performance-optimizations#using-dynamic-vertex-and-index-buffers).

## CPU-accessed meshes

The gameplay sample with V1 exposed another path:

```text
mesh update / triangle query
  -> 0x00833080
  -> 0x0070FE80
  -> vertex-buffer lock (return 0x0070852A)
     index-buffer lock  (return 0x0070887A)
```

The relevant callers are `0x008343C0` and `0x008334F0`. The latter leads to CPU reads of triangle indices and vertex positions. These accesses require preserved contents, so changing their locks to DISCARD would be incorrect.

V2 changes three more bytes:

| RVA | Original | V2 | Purpose |
| --- | --- | --- | --- |
| `0x4334B5` | `00` | `01` | Select the existing managed branch for the observed mesh factory |
| `0x30FF68` | `01` | `00` | Make managed vertex buffers in the paired-mesh class readable |
| `0x30FF77` | `01` | `00` | Make managed index buffers in that class readable |

The general creation wrappers already pass `dynamic=false`. Together with `managed=true` and `writeOnly=false`, the managed branch creates buffers with usage 0 in `D3DPOOL_MANAGED`. Other callers requesting dynamic allocation still use the original dynamic branch.

Managed resources retain a system-memory copy which the CPU can access; Direct3D uploads changes as required. This trades an extra copy/upload for avoiding GPU-synchronized CPU access. It is a targeted compatibility choice, not a recommendation to convert all frequently updated geometry. Microsoft warns about the cost of managed resources for rapidly changing data. [D3DPOOL](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dpool), [resource management](https://learn.microsoft.com/en-us/windows/win32/direct3d9/managing-resources), [D3DUSAGE](https://learn.microsoft.com/en-us/windows/win32/direct3d9/d3dusage).

## Limiter and scope

The main loop waits until 33 ms have elapsed since the iteration started, including the work performed during that iteration. It uses QueryPerformanceCounter in an active loop. The fix does not alter the timer, limiter, simulation, presentation interval or graphics settings.

In the sampled enclosure, 233 of 300 observations contained the active-limiter return. That does not rule out intermittent slow frames. Seventeen observations were in a Windows wait: nine in the mesh-lock path, seven in presentation and one in another render call. Stack candidates were checked against executable call sites; the sampler was not a complete stack unwinder or a frame-time benchmark.

## Verification

- Nine image signatures checked against the analysis copy.
- PE32 x86 DLL with the `Direct3DCreate9` export.
- Actual x86 detour executed with caller, dynamic/static, argument and stack checks; 10,000 repetitions.
- Original mesh constructor executed on mock objects, checking both buffer sizes, flags, retained dynamic branch and mismatch refusal; 10,000 constructions.
- Original Windows Direct3D object/vtable preserved by the proxy forwarding test.
- Installed V2 binary matched the reference checksum; its log confirmed both patches and successful Direct3D creation.
- Standalone graphics-device test unavailable in the restricted development environment: `CreateDevice` returned `0x8876086C` before any rendering. No GPU benchmark result is claimed from that test.

The original game executable and raw process logs are not distributed. The optional constructor regression test needs a local analysis copy supplied by the developer.

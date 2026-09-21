# Changelog

## V2 — current reference build

- Preserve the targeted V1 menu index-buffer correction.
- Use the engine's existing managed allocation branch for the observed CPU-accessed mesh factory.
- Make managed vertex/index buffers in that paired-mesh class readable, retaining their contents for CPU queries and updates.
- Keep the native 33 ms limiter and original Windows Direct3D objects.
- Validate nine code signatures before patching.

The reference DLL has SHA-256 `9bb09d0ca77b6259d8785323bdadfe556d925602837ec4a7439bd9e1c9eaf565`. This repository preserves that exact tested file.

## V1

- Apply DISCARD only to the dynamic UI index upload identified in the menu sample.
- Menu improvement reported during local testing; gameplay stalls remained and motivated V2.

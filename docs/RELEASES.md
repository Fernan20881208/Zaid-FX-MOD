# Release process

Zaid-FX-MOD uses GitHub Actions to build Android64 packages and publish versioned releases.

## Pull requests

Every pull request targeting `main` runs the Android64 build. The workflow uploads the generated `.geode` package as a temporary artifact when compilation succeeds.

A successful CI build confirms CMake configuration, C++ compilation, Android Media NDK linkage and package creation. It does not replace testing on a real Android device.

## Publishing a release

1. Update the version in `mod.json` and `CMakeLists.txt`.
2. Update `release-notes.md`, `changelog.md`, `README.md` and `about.md`.
3. Build the exact commit through GitHub Actions.
4. Install and test the generated artifact on supported Android64 devices.
5. Merge the validated pull request into `main`.
6. The `Build Android64` workflow creates a GitHub Release named after the version in `mod.json` and attaches the generated `.geode` file.

The workflow skips release creation when a release with the same version already exists. Never replace the asset of an existing release; create a new version instead.

## Version consistency

The following values must match before merging:

- `mod.json` version
- `CMakeLists.txt` project version
- heading in `release-notes.md`
- latest heading in `changelog.md`
- version documented in `README.md` and `about.md`

## Current release candidate

The current consolidated candidate is **v0.4.0 — ZaidLux**. It includes the two-pass visual pipeline, synchronized presets, color clipping correction and internal video-only MP4 recorder.

## Geode Index

A GitHub Release is not permission to submit to the Geode Index. The previous Index review explicitly requested no resubmission. Contact Index staff and obtain approval before using `geode index mods create` or attempting another submission.

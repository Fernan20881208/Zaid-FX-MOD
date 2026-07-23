# Release process

Zaid-FX-MOD uses immutable versioned releases built from GitHub Actions.

## Before merging

1. Confirm the version matches in `mod.json`, `CMakeLists.txt`, `changelog.md` and `release-notes.md`.
2. Confirm Android64 CI passes for the exact PR head.
3. Download and test the generated `.geode` artifact.
4. Compare the mod-disabled image against Effect Intensity = 0.
5. Test Default, Glow, ZaidLux, ZaidLux Neon, Cinematic, Cyberpunk and ZaidLux Performance.
6. Test the floating recorder in menus, level screens and gameplay.

## Publishing

A successful push to `main` builds the mod and creates a GitHub Release named after the `mod.json` version. The release uses `release-notes.md` and attaches the exact `.geode` produced by the build.

Do not replace an asset for an existing version. Increase the version and publish a new release instead.

## Geode Index

A GitHub Release does not authorize an Index submission. The previous review explicitly requested no resubmission. Contact Index staff and receive permission first.

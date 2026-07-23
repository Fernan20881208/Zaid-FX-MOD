# Geode Index publication

The previous Geode Index review explicitly requested that this mod not be resubmitted. Do not submit another package without first contacting Index staff and receiving confirmation that a new review is appropriate.

## Current state

The consolidated v0.4.0 source includes the ZaidLux visual pipeline, synchronized presets, privacy cleanup, documentation and internal video recorder. A successful GitHub Actions build verifies compilation and packaging only; it does not prove device compatibility or satisfy the Index review requirements.

## Preparation checklist

1. Build the exact source commit through GitHub Actions.
2. Install the generated `.geode` artifact on supported Android64 devices.
3. Complete the device tests listed in `ROADMAP.md`.
4. Confirm `mod.json`, `CMakeLists.txt`, `release-notes.md`, `changelog.md`, `README.md` and `about.md` describe the same version.
5. Merge only the tested commit into `main`.
6. Let the workflow publish a new immutable GitHub Release and `.geode` asset.
7. Confirm the release asset corresponds exactly to the reviewed source commit.
8. Keep private information, credentials, tokens and local paths out of the source tree and issue reports.
9. Contact Index staff and obtain explicit permission before another submission attempt.

## Commands

Only after Index staff confirms that resubmission is allowed:

```bash
geode index login
geode index mods create
```

For later approved versions:

```bash
geode index mods update
```

Use the direct URL of the versioned GitHub Release asset when prompted. Never replace an asset for an existing version because published package checksums must remain stable.

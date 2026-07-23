# Geode Index publication

Automatic updates inside Geode are provided by the official Geode Index. The index does not discover GitHub releases automatically; the developer must authenticate once and submit the first release.

## Initial submission

After `v0.2.0` is published and its `.geode` file is attached to GitHub Releases:

```bash
geode index login
geode index mods create
```

Use the direct release asset URL for `zaid.zaid-fx-mod.geode` when prompted.

An index administrator must approve the initial submission. Once approved, Zaid-FX-MOD appears in Geode's Download section and installed users receive update notifications directly in Geode.

## Future versions

For each update:

1. Increase `version` in `mod.json`.
2. Create a new GitHub Release without replacing an older package.
3. Run:

```bash
geode index mods update
```

The `.geode` file for an existing version must never be replaced because the index stores its checksum.

## Current repository preparation

- Public source repository.
- MIT license.
- Root `logo.png`.
- `about.md`, `support.md` and `changelog.md`.
- Versioned GitHub Releases with a direct `.geode` asset.
- Geometry Dash and Geode version metadata.
- Android64 build validation.

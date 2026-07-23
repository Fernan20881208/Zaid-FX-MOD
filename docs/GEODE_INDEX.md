# Geode Index publication

The previous Index review explicitly requested that this mod not be resubmitted. Do not submit another package without first contacting the Index staff and confirming that a new review is appropriate.

## Preparation checklist

1. Build the exact commit that will be submitted through GitHub Actions.
2. Test the generated `.geode` package on supported Android devices.
3. Confirm that the package version matches `mod.json`.
4. Publish a new GitHub Release; never replace an asset for an existing version.
5. Keep the source repository public and make sure the release package corresponds to the reviewed source.
6. Remove private contact details, credentials, tokens and local paths from the current tree.
7. Document known compatibility and performance limitations.

## Submission commands

After Index staff confirms that resubmission is allowed:

```bash
geode index login
geode index mods create
```

For later approved versions:

```bash
geode index mods update
```

Use the direct URL of the versioned `.geode` release asset when prompted.

# Zaid-FX-MOD

Android-first post-processing and visual filter framework for Geometry Dash using Geode.

> Status: early foundation. The current code provides Geode settings, packaged GLSL shaders, a reusable `CCGLProgram` wrapper and Android64 CI. It does **not** yet replace the game's final framebuffer.

## Planned effects

- Exposure, contrast, saturation and gamma
- Sharpening
- Vignette
- OLED, Vibrant, Cinematic, Competitive and “RTX Fake” presets
- Quarter-resolution bloom
- Dynamic quality based on frame time

“RTX Fake” means a stylized combination of bloom, tone mapping and sharpening. It is not hardware ray tracing.

## Supported target

- Geometry Dash 2.2081
- Geode 5.7.1
- Android ARM64 (`Android64` / `arm64-v8a`)

## Build

Install the Geode CLI, SDK binaries and Android NDK, then run:

```bash
geode sdk install-binaries -p android64
geode build -p android64
```

The package should be produced under `build-android64`.

## Repository structure

- `src/rendering`: shader program and future framebuffer pipeline
- `src/settings`: typed access to Geode settings
- `resources/shaders`: GLSL ES shaders packaged with the mod
- `.github/workflows`: Android64 build workflow
- `docs/ROADMAP.md`: implementation milestones and validation gates

## Safety and compatibility goals

- Never modify gameplay physics or hitboxes
- Restore OpenGL resources after Android context loss
- Avoid overriding Geometry Dash shader triggers
- Allow effects to be disabled immediately
- Prefer reduced-resolution multipass effects on mobile GPUs

## License

MIT

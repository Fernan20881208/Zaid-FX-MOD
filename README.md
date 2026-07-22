# Zaid-FX-MOD

Android-first post-processing and visual-filter framework for Geometry Dash using Geode.

> **v0.1.0 foundation:** the current build provides Geode settings, packaged GLSL shaders, a reusable `CCGLProgram` wrapper and verified Android64 CI. Global framebuffer post-processing is the next development milestone.

## Download and installation

Download the latest `.geode` package from the [GitHub Releases page](https://github.com/Fernan20881208/Zaid-FX-MOD/releases/latest), then import or place it in the Geode mods directory on Android.

Current target:

- Geometry Dash Android `2.2081`
- Geode `5.7.1`
- Android ARM64 (`Android64` / `arm64-v8a`)

## Planned effects

- Exposure, contrast, saturation and gamma
- Sharpening and vignette
- OLED, Vibrant, Cinematic, Competitive and **RTX Fake** presets
- Quarter-resolution bloom
- Dynamic quality based on frame time

**RTX Fake** means a stylized combination of bloom, tone mapping and sharpening. It is not hardware ray tracing.

## Developer and contact

**Zaid Navarro**

- Instagram: [@Zaid.nvr](https://www.instagram.com/Zaid.nvr/)
- WhatsApp: [+52 33 4515 8805](https://wa.me/523345158805)
- Email: [zaidnavarrosaucedo@gmail.com](mailto:zaidnavarrosaucedo@gmail.com)
- Source code and issues: [Fernan20881208/Zaid-FX-MOD](https://github.com/Fernan20881208/Zaid-FX-MOD)

These contact options are also exposed inside Geode: Instagram as the homepage, WhatsApp as the community link, GitHub as the source link, and the complete information in the Support tab.

## Build from source

Install the Geode CLI, SDK binaries and Android NDK, then run:

```bash
geode sdk install-binaries -p android64
geode build -p android64
```

The package should be produced under `build-android64`.

## Repository structure

- `src/rendering`: shader API and future framebuffer pipeline
- `src/settings`: typed access to Geode settings
- `resources/shaders`: packaged GLSL ES shaders
- `.github/workflows`: verified Android64 build and release workflow
- `docs/ROADMAP.md`: implementation milestones and validation gates

## Safety and compatibility goals

- Never modify gameplay physics or hitboxes
- Restore OpenGL resources after Android context loss
- Avoid overriding Geometry Dash shader triggers
- Allow effects to be disabled immediately
- Prefer reduced-resolution multipass effects on mobile GPUs

## License

MIT

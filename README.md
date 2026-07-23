# Zaid-FX-MOD

<p align="center">
  <img src="logo.png" width="220" alt="Zaid-FX-MOD logo">
</p>

Zaid-FX-MOD is an Android64 visual-effects mod for Geometry Dash built with Geode. It captures the final game frame immediately before presentation and applies the **ZaidLux** screen-space lighting pipeline without changing physics, hitboxes, level data or gameplay logic.

## Version 0.4.0

This release combines the work completed during the current development cycle:

- corrected preset application and persistence;
- two-pass post-processing with a reduced-resolution lighting buffer;
- restrained bloom and emissive lighting that preserve color instead of clipping the screen into magenta or yellow;
- simulated ambient occlusion, reflections, HDR, local contrast, specular highlights, light rays, depth separation and sharpen;
- reactive lighting for gameplay events;
- Low, Medium, High and Ultra quality levels;
- an internal Android H.264/MP4 screen recorder with **Record**, **Stop**, **Save** and **Delete** actions;
- removal of personal contact information, development-only controls and unused dependencies.

## Presets

- **Default** — disables post-processing and restores the original image.
- **Glow** — bloom-focused profile with mild emissive lighting.
- **ZaidLux** — balanced lighting, depth, highlights and restrained color grading.
- **ZaidLux Neon** — stronger emissive and electric colors while retaining highlight compression.
- **Cinematic** — darker contrast, stronger ambient depth and controlled saturation.
- **Cyberpunk** — cyan/magenta-biased lighting and stronger reflections.
- **ZaidLux Performance** — lower-cost profile for weaker devices.
- **Custom** — selected automatically after manually changing a control.

Preset values, toggles, quality and the custom preset are saved through Geode settings. The renderer and settings interface use the same configuration state.

## Effects

The settings are organized into Bloom, Emissive Lighting, Ambient Occlusion, Reflections, HDR, Color Grading, Local Contrast, Specular, Light Rays, Depth Blur, Sharpen, Reactive Lighting and Performance sections.

Expensive effects are skipped when disabled. Render targets and shaders are reused instead of being recreated every frame. The selected quality controls the reduced lighting-buffer resolution and shader sample count.

## Internal recorder

A **REC** button appears in the main menu.

1. Press **REC** and confirm recording.
2. Play normally; the recorder captures the final processed image.
3. Return to the menu and press **STOP**.
4. Choose **Save** to keep the MP4 or **Delete** to discard it.

Saved files are placed in the mod save directory under `recordings`. The current implementation records video only; game audio is not included. Recording resources are not allocated until recording begins.

## Rendering architecture

The mod hooks `CCEGLView::swapBuffers` through Geode. It copies the completed framebuffer to a reusable texture, renders lighting effects into a reduced-resolution framebuffer, then performs the final HDR, color, detail and finishing pass back to the presented framebuffer. OpenGL state is captured and restored around the operation.

ZaidLux uses stable screen-space approximations. It is not hardware ray tracing and does not have access to a traditional per-object depth buffer or material system.

## Compatibility and limitations

- Geometry Dash 2.2081 on Android64
- Geode 5.7.1
- Device GPU support and performance vary
- Screen-space category masks for player, objects and particles are approximations
- The recorder currently produces video-only MP4 files
- Shader behavior and MP4 playback still require testing on real Android devices

## Building

Install the Geode CLI and Android64 binaries, then run:

```bash
geode sdk install-binaries -p android64
geode build -p android64 --config Release
```

GitHub Actions compiles Android64 and uploads the generated `.geode` package. A successful push to `main` creates a versioned GitHub Release using `release-notes.md`.

## Releases and Index submission

See [`docs/RELEASES.md`](docs/RELEASES.md) for the release workflow. The previous Geode Index review explicitly requested that the mod not be resubmitted. Contact Index staff and receive permission before attempting another submission.

## Reporting problems

Use GitHub Issues and include the device model, GPU, Android version, Geometry Dash version, Geode version, selected preset and quality, steps to reproduce, and the latest crash or Geode log when relevant. Do not post private contact information, credentials or tokens.

## License

MIT

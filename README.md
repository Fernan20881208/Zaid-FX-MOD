# Zaid-FX-MOD

<p align="center">
  <img src="logo.png" width="220" alt="Zaid-FX-MOD logo">
</p>

Zaid-FX-MOD is an Android64 visual-effects mod for Geometry Dash built with Geode. It captures the final game frame immediately before presentation and applies the **ZaidLux** screen-space lighting pipeline without changing physics, hitboxes, level data or gameplay logic.

## Version 0.4.0

This release combines the current development cycle:

- synchronized preset application and persistence;
- two-pass post-processing with a reduced-resolution lighting-data buffer;
- a full-resolution source-first color pipeline that prevents blue or other unintended channel dominance;
- stable bloom, emissive lighting, ambient occlusion, reflections, HDR, local contrast, specular highlights, light rays, depth separation and sharpen;
- reactive lighting for gameplay events;
- Low, Medium, High and Ultra quality levels;
- removal of personal contact information, development-only controls and unused dependencies.

## Presets

- **Default** — disables post-processing and restores the original image.
- **Glow** — bloom-focused profile with mild emissive lighting.
- **ZaidLux** — balanced lighting, depth, highlights and restrained color grading.
- **ZaidLux Neon** — stronger emissive and electric colors with bounded saturation.
- **Cinematic** — darker contrast, stronger ambient depth and controlled saturation.
- **Cyberpunk** — intentional cyan/magenta styling without replacing the base image.
- **ZaidLux Performance** — lower-cost profile for weaker devices.
- **Custom** — selected automatically after manually changing a control.

Preset values, toggles, quality and the custom preset are saved through Geode settings. The renderer and settings interface use the same configuration state.

## Neutral output and pass-through testing

The final shader always begins with the untouched full-resolution framebuffer. The reduced lighting texture contains only additive light in RGB and ambient occlusion in alpha; it is never used as a replacement for the scene color.

Exposure, contrast, saturation, gamma, temperature, tint, highlights and shadows map their midpoint values to exact neutral operations. Setting **Effect Intensity** to `0` activates an exact pass-through path that outputs the captured framebuffer without color modification. Disabling all effects bypasses post-processing entirely.

Local contrast, depth blur and sharpen derive their detail samples from the original framebuffer rather than from bloom or emissive data. The renderer also blocks re-entrant processing so the shader cannot be applied twice during one presented frame.

## Effects

The settings are organized into Bloom, Emissive Lighting, Ambient Occlusion, Reflections, HDR, Color Grading, Local Contrast, Specular, Light Rays, Depth Blur, Sharpen, Reactive Lighting and Performance sections.

Expensive effects are skipped when disabled. Render targets and shaders are reused instead of being recreated every frame. The selected quality controls the reduced lighting-buffer resolution and shader sample count.

## Rendering architecture

The mod hooks `CCEGLView::swapBuffers` through Geode. It copies the completed framebuffer to a reusable full-resolution RGBA texture. Optional lighting effects are evaluated into a reduced-resolution RGBA target where RGB contains additive lighting and alpha contains ambient occlusion. The final pass starts from the original full-resolution texture, combines the optional lighting data, applies the enabled finishing stages and writes back to the presented framebuffer.

OpenGL program, framebuffer, texture, viewport, blend, depth, stencil, scissor, color mask and vertex-attribute state are captured and restored around the operation. Bounded debug logs record the active preset, framebuffer, target sizes and key color uniforms when the configuration changes.

ZaidLux uses stable screen-space approximations. It is not hardware ray tracing and does not have access to a traditional per-object depth buffer or material system.

## Compatibility and limitations

- Geometry Dash 2.2081 on Android64
- Geode 5.7.1
- Device GPU support and performance vary
- Screen-space category masks for player, objects and particles are approximations
- Final visual behavior should be tested on real Android devices

## Building

Install the Geode CLI and Android64 binaries, then run:

```bash
geode sdk install-binaries -p android64
geode build -p android64 --config Release
```

GitHub Actions validates release metadata, compiles Android64 and uploads the generated `.geode` package. A successful push to `main` creates a versioned GitHub Release using `release-notes.md`.

## Releases and Index submission

See [`docs/RELEASES.md`](docs/RELEASES.md) for the release workflow. The previous Geode Index review explicitly requested that the mod not be resubmitted. Contact Index staff and receive permission before attempting another submission.

## Reporting problems

Use GitHub Issues and include the device model, GPU, Android version, Geometry Dash version, Geode version, selected preset and quality, steps to reproduce, and the latest crash or Geode log when relevant. Do not post private contact information, credentials or tokens.

## License

MIT

# Zaid-FX-MOD

<p align="center">
  <img src="logo.png" width="220" alt="Zaid-FX-MOD logo">
</p>

Zaid-FX-MOD adds configurable color grading and lightweight screen effects to Geometry Dash on Android through Geode.

## Features

- Brightness, exposure, contrast, saturation and gamma controls
- Vignette, sharpening and chromatic aberration
- Lightweight highlight glow and tone mapping
- Adjustable effect intensity
- Default, Cinematic, Vibrant, Dark, Retro and Glow presets

Changing an individual slider switches the selected preset to **Custom**. Disabling the mod bypasses the post-processing pass immediately.

## How it works

The mod hooks `CCEGLView::swapBuffers` through Geode. Immediately before Android presents a frame, the current framebuffer is copied to a texture and drawn back through a single fullscreen shader pass.

The renderer preserves and restores the OpenGL state it changes. It does not modify gameplay logic, physics, hitboxes or level data.

## Limitations

- Android only
- The highlight glow is a lightweight single-pass approximation, not a full multi-pass bloom implementation
- The Glow preset is a color-grading profile; it does not provide ray tracing
- Results and performance depend on the device GPU and screen resolution

## Building

Install the Geode CLI, Android SDK dependencies and Android64 binaries, then run:

```bash
geode sdk install-binaries -p android64
geode build -p android64
```

## Reporting problems

Use the repository's GitHub Issues page. Include the device model, GPU, Android version, Geometry Dash version, Geode version and latest crash log when relevant.

## License

MIT

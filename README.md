# Zaid-FX-MOD

<p align="center">
  <img src="logo.png" width="220" alt="Zaid-FX-MOD logo">
</p>

Android-first final-frame post-processing and visual filters for Geometry Dash using Geode.

> **v0.2.0:** the mod intercepts the framebuffer immediately before Android presents it, copies the completed menu or gameplay frame to a texture, and draws a fullscreen GLSL quad back to the same framebuffer.

## Download and installation

Download the latest `.geode` package from the [GitHub Releases page](https://github.com/Fernan20881208/Zaid-FX-MOD/releases/latest), then import or place it in the Geode mods directory on Android.

Current target:

- Geometry Dash Android `2.2081`
- Geode `5.7.1`
- Android ARM64 (`Android64` / `arm64-v8a`)

## Final-frame rendering pipeline

The effect runs from `CCEGLView::swapBuffers`, after Geometry Dash has completed the frame and immediately before the platform presents it:

```text
Geometry Dash menu or PlayLayer
        ↓
Final bound framebuffer
        ↓ glCopyTexSubImage2D
Captured fullscreen texture
        ↓ GLSL program + uniforms
Fullscreen triangle-strip quad
        ↓
Same framebuffer
        ↓
swapBuffers
```

This avoids the previous problem where the original scene could be rendered outside the off-screen texture or drawn over the processed result.

## Live visual controls

Every control is sent to the shader on every processed frame:

- Effect intensity: `u_intensity`
- Brightness: `u_brightness`
- Exposure: `u_exposure`
- Contrast: `u_contrast`
- Saturation: `u_saturation`
- Gamma: `u_gamma`
- Vignette: `u_vignette`
- Sharpen: `u_sharpen`
- Pixel size: `u_texelSize`
- Red framebuffer test: `u_debugRed`

Saturation uses `0` for grayscale, `1` for the original color level and values above `1` for stronger colors. Exposure uses stops, so `-2` produces one quarter of the original light level before the remaining corrections.

Presets write their values into the real Geode settings. Moving an individual slider changes the selected preset to **Custom**, preventing a second configuration copy from replacing the slider value.

## Red framebuffer test

Enable **Enable effects**, then turn on **Red framebuffer test**. The complete menu or level should become solid red. This confirms that the final framebuffer hook, program, texture and fullscreen draw are connected.

After confirming the red test, disable it and test:

- Exposure `-2`: strongly darker image.
- Contrast `0.5`: visibly flatter image.
- Saturation `0`: grayscale.
- Disable effects: immediate return to the original image.

## Diagnostics

`Shader debug logging` records:

- Every `swapBuffers` render-hook call.
- Bound framebuffer and viewport dimensions.
- Shader compile and link errors.
- GLSL program ID and capture-texture ID.
- Every validated uniform and its value.
- Final fullscreen-quad draw confirmation.
- OpenGL errors from texture capture or drawing.

The renderer restores the previous program, framebuffer, viewport, scissor, blending, depth, stencil, culling, texture bindings, buffer bindings, write masks and vertex-attribute state before returning to Geometry Dash.

## Updates inside Geode

Geode delivers automatic in-app updates through the official **Geode Index**. GitHub releases alone are not enough. The repository and versioned release workflow are index-ready; after the one-time authenticated index submission is approved, future versions can be delivered through `geode index mods update` and will appear directly in Geode.

See [`docs/GEODE_INDEX.md`](docs/GEODE_INDEX.md) for the remaining authenticated submission step.

## Developer and contact

**Zaid Navarro**

- Instagram: [@Zaid.nvr](https://www.instagram.com/Zaid.nvr/)
- WhatsApp: [+52 33 4515 8805](https://wa.me/523345158805)
- Email: [zaidnavarrosaucedo@gmail.com](mailto:zaidnavarrosaucedo@gmail.com)
- Source code and issues: [Fernan20881208/Zaid-FX-MOD](https://github.com/Fernan20881208/Zaid-FX-MOD)

These contact options are also exposed inside Geode: Instagram as the homepage, WhatsApp as the community link, GitHub as the source link, and the complete information in the Support tab.

## Build from source

```bash
geode sdk install-binaries -p android64
geode build -p android64
```

## License

MIT

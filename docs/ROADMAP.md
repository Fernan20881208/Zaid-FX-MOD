# Roadmap

## Milestone 0 — Foundation

- [x] Geode Android64 project
- [x] Settings and presets
- [x] Packaged GLSL resources
- [x] `CCGLProgram` wrapper
- [x] Android64 CI

## Milestone 1 — Framebuffer proof of concept

- [ ] Identify a stable final-scene render hook on Android
- [ ] Capture one frame into an off-screen texture
- [ ] Draw a fullscreen quad without recursion
- [ ] Preserve and restore framebuffer, viewport and blend state
- [ ] Handle Android OpenGL context recreation

Validation gate: menus, normal levels, practice mode and editor render correctly after minimizing and restoring the app.

## Milestone 2 — Single-pass filters

- [ ] Connect settings to uniforms
- [ ] Exposure, contrast, saturation and gamma
- [ ] Sharpen and vignette
- [ ] Before/after toggle
- [ ] Per-level enable state

## Milestone 3 — Bloom

- [ ] Bright-pass extraction
- [ ] Quarter-resolution render targets
- [ ] Horizontal and vertical blur
- [ ] Composite pass
- [ ] Dynamic quality fallback

## Milestone 4 — Presets and compatibility

- [ ] OLED
- [ ] Vibrant
- [ ] Cinematic
- [ ] Competitive
- [ ] RTX Fake
- [ ] Compatibility testing with native Geometry Dash shader triggers

# Roadmap

## Completed in v0.4.0

- [x] Android64 Geode project and CI build
- [x] Final-frame hook through `CCEGLView::swapBuffers`
- [x] Reusable framebuffer capture and OpenGL state restoration
- [x] Two-pass ZaidLux post-processing pipeline
- [x] Reduced-resolution lighting buffer and four quality levels
- [x] Bloom, emissive lighting, AO, reflections and light rays
- [x] Fake HDR, color grading, local contrast, depth separation and sharpen
- [x] Reactive gameplay lighting
- [x] Preset synchronization and persistent custom settings
- [x] Internal H.264/MP4 video recorder with save/delete flow
- [x] Current-tree privacy cleanup and documentation consolidation
- [x] Automated Android64 artifact generation and versioned release workflow

## Required device validation

- [ ] Test Default, Glow, ZaidLux, ZaidLux Neon, Cinematic, Cyberpunk and ZaidLux Performance on a real Android64 device
- [ ] Confirm the corrected shaders no longer clip saturated backgrounds into solid magenta or yellow
- [ ] Test menus, gameplay, practice mode, editor, pause, restart and level changes
- [ ] Test minimizing and restoring the app and changing resolution/fullscreen state when available
- [ ] Test at least two Android GPU families when possible
- [ ] Measure FPS and memory use for Low, Medium, High and Ultra
- [ ] Record, stop, save, delete and play generated MP4 files
- [ ] Verify recorder orientation, duration, frame pacing and output paths
- [ ] Document incompatible shader, texture or capture mods

## Before another Geode Index submission

- [ ] Complete the required device validation
- [ ] Keep the pull request as draft until the generated package is tested
- [ ] Publish a unique versioned GitHub Release without replacing an older asset
- [ ] Confirm the release package matches the reviewed source commit
- [ ] Contact Geode Index staff and receive permission before resubmitting

## Possible future work

- Audio capture and A/V synchronization for the internal recorder
- Hardware-surface encoding to avoid CPU RGBA-to-YUV conversion
- Automatic quality selection based on measured frame time
- Per-scene or per-level profiles
- Improved player/object masks where stable engine hooks are available
- Additional GPU compatibility testing and shader fallbacks

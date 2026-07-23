# Zaid-FX-MOD v0.4.0 — ZaidLux

This release combines the complete ZaidLux visual pipeline, synchronized presets, a neutral full-resolution color path and the first internal Android screen recorder.

## Highlights

- Two-pass post-processing with Low, Medium, High and Ultra quality levels
- Bloom, emissive lighting, ambient occlusion, reflections and light rays
- Fake HDR, full color grading, local contrast, specular highlights, depth separation and sharpen
- Reactive lighting for gameplay events
- Default, Glow, ZaidLux, ZaidLux Neon, Cinematic, Cyberpunk and ZaidLux Performance presets
- Persistent sliders, toggles, quality and custom preset values
- Final color processing now always begins from the untouched full-resolution framebuffer
- Reduced lighting target stores only additive lighting and ambient occlusion, preventing colored downsampling from replacing the base scene
- Local contrast, depth blur and sharpen sample the original framebuffer instead of the lighting texture
- Exact neutral midpoints and an Effect Intensity = 0 pass-through validation path
- Re-entry protection guarantees one shader application per presented frame
- Rebalanced presets and bounded output to prevent blue, magenta, yellow or cyan channel dominance
- Floating recorder overlay in menus, level screens and gameplay
- Internal H.264/MP4 recorder with Record, Stop, Save and Delete flow
- Current project-tree privacy and maintenance cleanup

## Recorder limitation

The internal recorder captures video only. Game audio is not included. Saved MP4 files are placed in the mod save directory under `recordings`.

## Compatibility

Built for Geometry Dash 2.2081, Geode 5.7.1 and Android64. The code, Android Media NDK linkage and package generation are validated in CI. Final visual behavior, recorder output and performance should still be checked on real devices and multiple GPU families.

## Geode Index

Do not resubmit this mod to the Geode Index without first contacting Index staff and receiving permission, because the previous review explicitly requested no resubmission.

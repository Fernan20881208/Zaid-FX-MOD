# v0.4.0 — ZaidLux

## Rendering and visual effects

- Replaced the original single-pass color filter with a two-pass post-processing pipeline.
- Added a reusable reduced-resolution lighting framebuffer controlled by Low, Medium, High and Ultra quality levels.
- Added advanced bloom controls for intensity, threshold, radius, soft knee and quality.
- Added luminance- and color-based emissive lighting with player, object and particle approximations.
- Added screen-space ambient occlusion and reflection approximations.
- Added simulated HDR with dynamic range, highlight compression, shadow recovery and level controls.
- Added complete color grading, local contrast, specular highlights, light rays, depth separation and thresholded sharpen.
- Added reactive lighting for music, jumps, orbs, portals, coins, speed and death.
- Corrected excessive magenta/yellow clipping by compressing light energy, preserving hue and blending extreme highlights toward neutral white.
- Reduced default preset saturation, vibrance and flash strength for safer visual output.
- Preserved and restored the OpenGL state used by Geometry Dash around every processed frame.

## Presets and settings

- Added Default, Glow, ZaidLux, ZaidLux Neon, Cinematic, Cyberpunk and ZaidLux Performance presets.
- Fixed presets so they update the real shader values, visible sliders, toggles and quality immediately.
- Prevented stale setting callbacks from overwriting a newly selected preset.
- Added persistent custom presets, section reset and full reset actions.
- Unified the settings interface and renderer around one configuration source.

## Internal recorder

- Added a REC button to the main menu.
- Added Android MediaCodec H.264 encoding and MediaMuxer MP4 output.
- Added Record, Stop, Save and Delete flow.
- Captures the final processed framebuffer so videos match the image shown in game.
- Saves retained videos under the mod save directory in `recordings`.
- Allocates recorder resources only while recording.
- The current recorder captures video only and does not include game audio.

## Project cleanup

- Removed personal contact information from current metadata and documentation.
- Removed development-only diagnostics, public debug controls and the unused Node IDs dependency.
- Reduced routine logging while preserving actionable shader, OpenGL and recorder errors.
- Updated the README, about, support, roadmap, Index guidance and release workflow.
- Added automated Android64 package generation and versioned release publishing after successful pushes to `main`.

# v0.3.0

- Removed personal contact information from the current project tree.
- Replaced the misleading RTX preset name with Glow.
- Simplified preset application and renderer state management.
- Removed the menu diagnostic button and unused development settings.

# v0.2.1

- Fixed the original preset application flow.
- Added initial highlight glow, chromatic aberration and tone mapping controls.

# v0.2.0

- Moved post-processing to `CCEGLView::swapBuffers` immediately before frame presentation.
- Added framebuffer capture and a fullscreen shader pass.

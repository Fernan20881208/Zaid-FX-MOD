# v0.3.0

- Removed personal contact information from the mod metadata and documentation.
- Replaced the misleading RTX preset name with Glow.
- Removed the menu diagnostic button, public debug settings and the unused Node IDs dependency.
- Simplified preset application and renderer state management.
- Replaced direct filesystem string conversion with Geode file and path utilities.
- Reduced routine logging while preserving actionable shader and OpenGL errors.
- Preserved the source frame alpha in the final shader output.
- Clarified that highlight glow is a lightweight single-pass effect.

# v0.2.1

- Fixed preset application and added Default, Cinematic, Vibrant, Dark, Retro and experimental glow-focused profiles.
- Added highlight glow, chromatic aberration and tone mapping controls.
- Added a fallback icon for the temporary in-game status button.

# v0.2.0

- Moved post-processing to `CCEGLView::swapBuffers` immediately before frame presentation.
- Added framebuffer capture and a fullscreen shader pass.
- Added shader diagnostics for development.

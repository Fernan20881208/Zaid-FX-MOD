# v0.2.0

- Moved post-processing to `CCEGLView::swapBuffers`, after the complete menu or gameplay frame has been rendered and immediately before Android presents it.
- Copies the final bound framebuffer into a fullscreen texture with `glCopyTexSubImage2D`.
- Draws the captured texture back to the same framebuffer with a clip-space GLSL quad.
- Prevents the original unprocessed scene from being drawn over the result.
- Restores the previous framebuffer, program, viewport, scissor, blending, depth, stencil, culling, textures, buffers, write masks and vertex-attribute state.
- Added shader compilation and linking diagnostics with the final GLSL program ID.
- Validates every uniform location and never writes to location `-1`.
- Updates exposure, brightness, contrast, saturation, gamma, vignette, sharpen and global intensity every processed frame.
- Added **Red framebuffer test** to force the complete screen to solid red and prove that the presentation pipeline is connected.
- Connected **Enable effects** directly to the final-frame pass; disabling it immediately restores the original game output.
- Added the official Zaid-FX-MOD logo to Geode and the menu button.
- Added versioned GitHub Release automation and Geode Index submission instructions for future in-app updates.
- Retained one authoritative settings state and live slider-to-uniform logging.

# v0.1.0

- Initialized the Android64 Geode project.
- Added configurable FX settings.
- Added a reusable `CCGLProgram` wrapper.
- Added GLSL ES passthrough and color-grading shaders.
- Added Android64 GitHub Actions configuration.
- Added the post-processing implementation roadmap.
- Added Zaid Navarro's public contact information to the README and Geode mod pages.
- Added Geode links for Instagram, GitHub and WhatsApp, plus a Support tab with email contact.

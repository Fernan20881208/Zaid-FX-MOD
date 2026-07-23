# v0.2.1

- Fixed presets that previously changed only the selected text without reliably applying a visible result.
- Added one central `applyPreset(presetId)` flow that loads all values, updates Geode settings and sliders, updates the active renderer state and queues uniform updates immediately.
- Added clearly differentiated Default, Cinematic, Vibrant, Dark, Retro and RTX profiles.
- Prevented delayed setting events from switching a freshly applied preset to Custom when the values still match that preset.
- Manual slider changes now switch the preset label to Custom without disabling effects.
- Added lightweight bloom, chromatic aberration and ACES-style tone mapping controls and GLSL uniforms.
- Added temporary logs for selected preset, internal ID, loaded values, assigned sliders, renderer values, uniform updates and final shader refresh.
- Replaced the previous logo resources with the exact image supplied by Zaid Navarro, center-cropped without stretching.
- Retained a safe fallback icon if the in-game logo texture cannot be loaded.

# v0.2.0

- Moved post-processing to `CCEGLView::swapBuffers`, after the complete menu or gameplay frame has been rendered and immediately before Android presents it.
- Copies the final bound framebuffer into a fullscreen texture with `glCopyTexSubImage2D`.
- Draws the captured texture back to the same framebuffer with a clip-space GLSL quad.
- Added shader diagnostics and the red framebuffer test.

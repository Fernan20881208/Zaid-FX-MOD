# v0.2.0

- Added a functional fullscreen post-processing pipeline around `CCDirector::drawScene`.
- Connected every Geode slider to one authoritative renderer settings state.
- Added immediate typed setting listeners for intensity, brightness, exposure, contrast, saturation, gamma, vignette and sharpen.
- Updated all shader uniforms during every processed frame.
- Added effect-intensity blending and brightness to the GLSL shader.
- Corrected value ranges and clamping before values reach OpenGL.
- Made presets update the real slider values and switch to `Custom` after manual edits.
- Added temporary slider, renderer, uniform and final-frame diagnostic logs.
- Added cached uniform lookup and missing-uniform validation.
- Added safe fallback to normal rendering when the pipeline cannot be created.
- Verified successful Android64 compilation and `.geode` packaging in GitHub Actions.

# v0.1.0

- Initialized the Android64 Geode project.
- Added configurable FX settings.
- Added a reusable `CCGLProgram` wrapper.
- Added GLSL ES passthrough and color-grading shaders.
- Added Android64 GitHub Actions configuration.
- Added the post-processing implementation roadmap.
- Added Zaid Navarro's public contact information to the README and Geode mod pages.
- Added Geode links for Instagram, GitHub and WhatsApp, plus a Support tab with email contact.

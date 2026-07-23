# Zaid-FX-MOD v0.4.0 — ZaidLux

Zaid-FX-MOD applies configurable visual post-processing to Geometry Dash on Android64 through Geode. The mod is visual only and does not modify physics, hitboxes, level data or gameplay logic.

## Corrected rendering pipeline

The completed game frame is copied into a full-resolution RGBA texture immediately before presentation. Optional lighting effects are rendered into a separate reduced-resolution target where RGB stores additive lighting and alpha stores ambient occlusion. The final shader always begins from the untouched full-resolution framebuffer, then combines only the enabled effects.

Neutral midpoint values for exposure, contrast, saturation, gamma, temperature, tint, highlights and shadows preserve the original image. Setting Effect Intensity to zero activates an exact pass-through path. A re-entry guard prevents the post-process shader from being applied twice to the same presented frame.

## Presets

Default, Glow, ZaidLux, ZaidLux Neon, Cinematic, Cyberpunk and ZaidLux Performance are included. Manual changes switch the profile to Custom while preserving all values.

## Floating recorder

The internal video recorder appears as a floating top-right overlay in the main menu, level browser, level selector and gameplay. It does not participate in the normal menu layout. REC starts capture, STOP finalizes it, and SAVE lets the user keep or delete the temporary MP4.

The recorder currently captures H.264 video without game audio. Saved files are stored under the mod save directory in `recordings`.

For bug reports, use the repository Issues page and include device, GPU, Android, Geometry Dash and Geode versions, selected preset and quality, and relevant logs.

# Zaid-FX-MOD

Zaid-FX-MOD is an Android-focused visual-effects framework for Geometry Dash and Geode.

Version 0.2.0 captures the complete rendered frame into an off-screen texture and applies a live GLSL post-processing pass before displaying it. Exposure, brightness, contrast, saturation, gamma, vignette, sharpening and global effect intensity update while the game is running.

The renderer uses one shared, sanitized settings state. Presets update the same Geode settings used by the sliders and shader, preventing duplicate configuration values from overriding manual changes.

Temporary diagnostic logging can report slider output, renderer input, uniform updates and the final values used during rendering.

The project does not claim to provide real RTX or ray tracing. The “RTX Fake” preset is a mobile-friendly visual style based on color grading, contrast and sharpening; bloom and other multipass effects remain planned.

## Developer

**Zaid Navarro**

## Contact

- Instagram: [@Zaid.nvr](https://www.instagram.com/Zaid.nvr/)
- WhatsApp: [+52 33 4515 8805](https://wa.me/523345158805)
- Email: [zaidnavarrosaucedo@gmail.com](mailto:zaidnavarrosaucedo@gmail.com)
- Source and issues: [GitHub repository](https://github.com/Fernan20881208/Zaid-FX-MOD)

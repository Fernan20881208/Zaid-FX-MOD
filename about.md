# Zaid-FX-MOD

Zaid-FX-MOD is an Android-focused final-frame visual-effects mod for Geometry Dash and Geode.

Version 0.2.0 processes the framebuffer immediately before Android presents it. The completed menu or gameplay frame is copied into a texture, passed through the mod's GLSL program, and drawn back with a fullscreen quad before `swapBuffers`.

Exposure, brightness, contrast, saturation, gamma, vignette, sharpening and global effect intensity update while the game is running. Turning **Enable effects** off bypasses the post-processing pass immediately.

## Pipeline test

Enable **Red framebuffer test** to force the complete final screen to solid red. This validates the render hook, capture texture, GLSL program and fullscreen draw. Disable it afterwards to use the normal effects.

Diagnostic logging reports the render hook, framebuffer, viewport, program ID, texture ID, uniform values, shader compile/link errors and final quad draw.

The project does not claim to provide real RTX or ray tracing. The **RTX Fake** preset is a mobile-friendly visual style based on color grading, contrast and sharpening; bloom and other multipass effects remain planned.

## Updates

Automatic in-app updates will be delivered through the official Geode Index after the initial authenticated submission and approval. GitHub Releases remain the source of the versioned `.geode` packages.

## Developer

**Zaid Navarro**

## Contact

- Instagram: [@Zaid.nvr](https://www.instagram.com/Zaid.nvr/)
- WhatsApp: [+52 33 4515 8805](https://wa.me/523345158805)
- Email: [zaidnavarrosaucedo@gmail.com](mailto:zaidnavarrosaucedo@gmail.com)
- Source and issues: [GitHub repository](https://github.com/Fernan20881208/Zaid-FX-MOD)

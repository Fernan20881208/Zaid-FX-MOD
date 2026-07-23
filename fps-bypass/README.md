# Zaid FPS Bypass

An Android64 Geode mod for Geometry Dash that requests a custom game rate and the closest compatible Android display mode between 30 and 360 FPS.

## Settings

- **Enable FPS bypass** — applies or removes the game and Android display requests immediately.
- **Target FPS** — configurable from 30 to 360 FPS.

When enabled, the mod:

1. updates the Geometry Dash/Cocos2d animation interval;
2. finds the supported Android display mode closest to the requested FPS while preserving the current resolution when possible;
3. applies that display mode ID to the game window;
4. requests the same rate from the active `GLSurfaceView` surface.

When disabled, it clears the Android frame-rate preference and restores the latest animation interval requested by Geometry Dash. The selected value is stored through Geode settings.

## Important limitations

Android treats display mode and surface frame-rate requests as preferences. A phone may still refuse a mode because of battery saver, thermal throttling, OEM game settings, resolution restrictions or another system policy.

The mod logs the selected Android mode. If 144 Hz is exposed, the log should contain a line similar to:

```text
[ZaidFPS] requested 144 FPS; selected Android mode ... @ 144.00 Hz
```

Selecting a rate above the highest mode exposed by Android does not create additional visible frames.

## Compatibility

- Geometry Dash 2.2081
- Geode 5.7.1
- Android64
- Display mode selection requires Android 6.0 or newer
- Direct surface frame-rate requests require Android 11 or newer

## Build

```bash
geode sdk install-binaries -p android64
geode build -p android64 --config Release
```

Run the command from this `fps-bypass` directory.

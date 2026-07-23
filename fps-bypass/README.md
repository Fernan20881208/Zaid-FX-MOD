# Zaid FPS Bypass

A small Android64 Geode mod for Geometry Dash that lets the user request a custom frame rate between 30 and 360 FPS.

## Settings

- **Enable FPS bypass** — applies or removes the override immediately.
- **Target FPS** — configurable from 30 to 360 FPS.

When disabled, the mod restores the latest animation interval requested by Geometry Dash. The selected value is stored through Geode settings.

## Important limitation

The mod changes the animation interval requested by the game. Actual visible FPS is still limited by the phone display refresh rate, Android, thermal throttling and device performance. Selecting 240 FPS on a 120 Hz screen does not make the display show 240 unique frames.

## Compatibility

- Geometry Dash 2.2081
- Geode 5.7.1
- Android64

## Build

```bash
geode sdk install-binaries -p android64
geode build -p android64 --config Release
```

Run the command from this `fps-bypass` directory.

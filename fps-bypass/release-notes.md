# Zaid FPS Bypass v1.0.1

Android display refresh-rate fix.

- Requests the supported Android display mode closest to the selected FPS
- Preserves the current screen resolution when a matching high-refresh mode exists
- Applies `preferredDisplayModeId` and `preferredRefreshRate` to the game window
- Requests the selected rate from the active render surface on Android 11+
- Uses an explicit frame-rate change strategy on Android 12+
- Clears the Android display preference when the bypass is disabled
- Logs the requested FPS and the Android mode selected by the device
- Keeps the existing 30–360 FPS selector and persistent settings

Android can still reject a requested mode because of device policy, battery saver, thermal throttling or OEM game settings.

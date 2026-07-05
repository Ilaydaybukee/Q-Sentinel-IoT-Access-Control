# Q-Sentinel V4 Pro - VS Code / PlatformIO

This folder is prepared for VS Code with PlatformIO.

## How to Open

1. Open VS Code.
2. Install the PlatformIO extension.
3. Open the `firmware/platformio` folder.
4. Edit Wi-Fi credentials in `include/config.h`.
5. Run `Build` and then `Upload` from PlatformIO.
6. Open Serial Monitor at `115200` baud.

## PlatformIO Commands

```bash
pio run
pio run -t upload
pio device monitor -b 115200
```

# eyes (bring-up examples)

This folder contains focused ESP-IDF projects to bring up the GC9D01 round LCD(s) and then run the eye animation.

Recommended progression:
- `01_color_bars_idf`: solid fills / bars to validate SPI + color order + rotation + backlight.
- `02_eyetest`: single display (left eye only) stability test.
- `03_eyestest`: dual display (both eyes) stability test.
- `04_eyes`: final app entry point (still display-only for now).

Legacy/extra (safe to ignore if you follow the progression above):
- `02_eye_animation_idf`
- `03_uncannyeyes_idf`

The required component is vendored here:
- `components/display_eyes`

## Copy into your grokbot repo

If you want these under `D:\\esp32_projects\\grokbot\\examples`, you can copy the whole folder:

- `robocopy D:\\esp32s3\\examples\\eyes D:\\esp32_projects\\grokbot\\examples\\eyes /E`

## Build / flash

From an ESP-IDF PowerShell (after running `export.ps1`):

- `cd <project>`
- `idf.py set-target esp32p4`
- `idf.py build`
- `idf.py -p COM5 flash monitor`

## Backlight debug

If you get a black screen/no backlight, flip the macro in the project `main.c`:

- `#define EYE_BACKLIGHT_ACTIVE_LOW 1`

Some boards wire BL as active-low.

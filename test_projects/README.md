# Grokbot Test Projects

This folder contains ESP32 test projects for various grokbot subsystems and hardware components.

## Projects

### chat_wifi (ESP32-P4)
ESP32-P4 WiFi + Audio test firmware with ES8311 codec integration. Tests WiFi connectivity and audio I/O functionality for voice assistant features.

**Hardware:** ESP32-P4 with ES8311 audio codec  
**Status:** Built and ready (674KB binary), refactored to use ES8311 driver library  

### eyes (ESP32-S3)
Multiple eye animation projects using GC9A01 round TFT displays. Various iterations testing different animation approaches and display configurations.

**Hardware:** ESP32-S3 with dual GC9A01 240x240 round displays  
**Projects:**
- `01_color_bars_idf` - Basic color bar test
- `02_eye_animation_idf` - Simple animation test
- `03_eyestest` / `03_uncannyeyes_idf` - Uncanny eyes animations
- `04_eyes` / `05_eyes` - Advanced eye tracking and blinking

### ears (ESP32-S3)
Audio input processing project for voice recognition and sound detection.

**Hardware:** ESP32-S3 with microphone array  
**Status:** Audio input testing

### slave (ESP32)
Secondary ESP32 controller for distributed processing or auxiliary functions.

**Hardware:** ESP32  
**Status:** Development

## Building Projects

Each project directory contains standard ESP-IDF structure:
```bash
cd <project_name>
idf.py set-target <esp32|esp32s3|esp32p4>
idf.py build
idf.py -p COMx flash monitor
```

## Notes

- All projects use ESP-IDF v6.1-dev
- Python environment configured at `~/.espressif/`  
- Main grokbot firmware is in `../firmware/`
- These are test/development projects, not production code

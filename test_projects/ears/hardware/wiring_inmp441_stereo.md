# INMP441 Stereo Wiring for ESP32-P4 Nano

## Hardware Setup for Sound Localization

### Shared I2S Bus (both mics connect to same pins)
| Signal | ESP32-P4 GPIO | Notes |
|--------|---------------|-------|
| BCLK   | GPIO4         | Bit clock (shared) |
| WS     | GPIO5         | Word select / LRCLK (shared) |
| SD     | GPIO6         | Data (shared - TDM multiplexed) |
| VDD    | 3.3V          | Power supply |
| GND    | GND           | Ground |

### Left Microphone
| INMP441 Pin | Connection |
|-------------|------------|
| VDD         | 3.3V       |
| GND         | GND        |
| SD          | GPIO6      |
| L/R         | **GND**    |
| WS          | GPIO5      |
| SCK         | GPIO4      |

### Right Microphone  
| INMP441 Pin | Connection |
|-------------|------------|
| VDD         | 3.3V       |
| GND         | GND        |
| SD          | GPIO6      |
| L/R         | **3.3V**   |
| WS          | GPIO5      |
| SCK         | GPIO4      |

## How It Works

Both INMP441 mics share the same I2S data line (SD → GPIO6). They take turns transmitting:
- When WS is LOW: Left mic (L/R→GND) transmits its data
- When WS is HIGH: Right mic (L/R→VCC) transmits its data

This is standard I2S stereo operation - no extra pins needed!

## Physical Placement

```
     Sound Source
          |
          v
    ┌─────────────┐
    │             │
   [L]    10cm   [R]
   MIC ◄───────► MIC
    │             │
    └──────┬──────┘
           │
       ESP32-P4
```

- Mount mics **10cm apart** (adjustable in firmware: MIC_SPACING_M)
- Face mics **forward** (same direction)
- Keep mics at **same height**
- Avoid obstructions between mics

## Localization Algorithm

1. Capture stereo audio (512 samples per channel)
2. Cross-correlate left and right channels
3. Find lag (delay) at maximum correlation
4. Convert lag to angle using:
   - `delay_seconds = lag / sample_rate`
   - `distance_diff = delay * speed_of_sound`
   - `angle = arcsin(distance_diff / mic_spacing)`

## Expected Output

```
L[-----------|----*------]R  Angle: +15.3°  Lag:  +4  Level: -25.1 dB
```
- `*` shows estimated direction
- `|` marks center (0°)
- Positive angle = sound from LEFT
- Negative angle = sound from RIGHT

## Build & Flash

```bash
cd /home/mopar/projects/ears/firmware/02_stereo_localization
source ~/esp/esp-idf/export.sh
idf.py build
```

Flash from Windows PowerShell:
```powershell
python -m esptool --chip esp32p4 -p COM5 -b 460800 write-flash --flash-mode dio --flash-size 2MB --flash-freq 80m 0x2000 \\wsl$\Ubuntu\home\mopar\projects\ears\firmware\02_stereo_localization\build\bootloader\bootloader.bin 0x8000 \\wsl$\Ubuntu\home\mopar\projects\ears\firmware\02_stereo_localization\build\partition_table\partition-table.bin 0x10000 \\wsl$\Ubuntu\home\mopar\projects\ears\firmware\02_stereo_localization\build\stereo_localization.bin
```

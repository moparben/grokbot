# Grokbot Project Status - Dec 16, 2025

## Project Overview
- **Project**: Grokbot - ESP32-P4 based robot with IMU, motors, LEDs, WiFi, Audio
- **Hardware**: Waveshare ESP32-P4-NANO, Adafruit BNO085 IMU, ES8311 Audio Codec
- **Firmware Path**: `D:\esp32_projects\grokbot\firmware`
- **Build Path**: `D:\esp32_projects\grokbot\build`

## ESP-IDF Environment
- **Version**: v5.5.1
- **IDF Path**: `D:\esp32_projects\Espressif\frameworks\esp-idf-v5.5.1`
- **Python env**: `D:\esp32_projects\Espressif\python_env\idf5.5_py3.13_env`
- **Note**: Old `IDF_PATH` env var points to `C:\esp32_projects\esp_idf` - can be deleted or updated

## 🎉 MAJOR MILESTONE: ES8311 AUDIO DRIVER COMPLETE ✅

### What Works
- ✅ ES8311 codec initialization (chip ID 0x83 verified)
- ✅ I2S output at 16kHz stereo, 16-bit
- ✅ Tone generation (100-8000 Hz)
- ✅ Startup melody
- ✅ Happy/success melody
- ✅ Error sound
- ✅ Volume control 0-100%
- ✅ WAV file playback
- ✅ PA enable/disable (GPIO 53)

### Audio Configuration (VERIFIED)
| Setting | Value | Notes |
|---------|-------|-------|
| Sample Rate | 16kHz | Good for voice/TTS |
| MCLK | 6.144MHz | **MUST be 384x multiplier** |
| I2C Address | 0x18 | ES8311 codec |
| PA GPIO | 53 | NS4150B amp, active HIGH |

### Audio Component Location
```
firmware/components/drivers/audio/
├── audio_driver.c       # Implementation
├── include/
│   └── audio_driver.h   # Public API
└── CMakeLists.txt
```

### Example Project Location
Working standalone test: `examples/audio-test/main/audio_test_main.c`

## Hardware Configuration (from board_config.h - UPDATED Dec 16)
| Component | GPIO | Notes |
|-----------|------|-------|
| Status LED | 52 | Heartbeat LED |
| Eye LED | 38 | WS2812 RGB |
| **Audio I2C SDA** | **7** | Shared with touch |
| **Audio I2C SCL** | **8** | ES8311 @ 0x18 |
| **Audio I2S MCLK** | **13** | 6.144MHz (384x) |
| **Audio I2S SCLK** | **12** | Bit clock |
| **Audio I2S LRCK** | **10** | Word select |
| **Audio I2S DIN** | **9** | Mic input |
| **Audio I2S DOUT** | **11** | Speaker output |
| **Audio PA Enable** | **53** | NS4150B amp (HIGH=on) |
| BNO085 TX | 20 | Changed from 37 |
| BNO085 RX | 6 | Changed from 38 |
| BNO085 Baud | 115200 | |

## Issue #1: Console Corruption - FIXED ✅
**Problem**: Console output was garbled (`TX��� N� N� N`) when BNO085 UART initialized

**Root Cause**: GPIO 38 pin conflict
- `EYE_LED_GPIO` was defined as GPIO 38
- `BNO085_UART_RX_GPIO` was also GPIO 38
- Both drivers fighting for same pin

**Solution**: Changed BNO085 UART pins in `board_config.h`:
```c
#define BNO085_UART_TX_GPIO     20   // was 37
#define BNO085_UART_RX_GPIO     6    // was 38 (conflicted with EYE_LED)
```

**Result**: Console output is now clean ✅

## Issue #2: BNO085 Not Responding - IN PROGRESS 🔄
**Current Error**:
```
E (3837) BNO085_UART: No response from sensor - check wiring and P1 pin!
E (3837) BNO085_UART:   For UART mode: P1 must be connected to 3.3V
```

**User Verified**:
- ✅ PS1 pin on BNO085 has 3.3V (measured with Fluke 87)
- ✅ User swapped TX/RX wires (should be crossed)

**Required Wiring for UART Mode**:
| ESP32-P4 | BNO085 | Notes |
|----------|--------|-------|
| GPIO 20 (TX) | RX | TX→RX crossed |
| GPIO 6 (RX) | TX | RX→TX crossed |
| 3.3V | VIN | Power |
| GND | GND | Ground |
| 3.3V | PS1 | UART mode select |

**Possible Issues to Check**:
1. TX/RX still not crossed correctly
2. Bad wire/connection
3. BNO085 needs power cycle after PS1 change
4. Baud rate mismatch (try 3000000)

## COM Port
- Device: USB-Enhanced-SERIAL CH343
- Usually COM5 or COM7 (check Device Manager after restart)

## Flash Commands
```powershell
# One-liner flash command (replace COM7 with your port)
Push-Location D:\esp32_projects\grokbot\firmware; D:\esp32_projects\Espressif\frameworks\esp-idf-v5.5.1\export.ps1; idf.py flash monitor -p COM7; Pop-Location

# Or step by step:
cd D:\esp32_projects\grokbot\firmware
D:\esp32_projects\Espressif\frameworks\esp-idf-v5.5.1\export.ps1
idf.py flash monitor -p COM7
```

## Key Files Modified
1. **board_config.h** (`D:\esp32_projects\grokbot\firmware\main\board_config.h`)
   - Changed `BNO085_UART_TX_GPIO` from 37 to 20
   - Changed `BNO085_UART_RX_GPIO` from 38 to 6

2. **subsystems.c** - Init order changed (sensors before I2C) - didn't fix issue but still in place

## Debug Session History
1. Started with console corruption when BNO085 UART init
2. Tried reordering init (sensors before I2C) - didn't help
3. Discovered GPIO 38 pin conflict between EYE_LED and BNO085_UART_RX
4. Changed BNO085 pins to GPIO 20/6 - **fixed console corruption**
5. User rewired BNO085 to new pins
6. BNO085 still not responding - "No response from sensor"
7. User confirmed PS1 pin has 3.3V
8. User swapped TX/RX wires
9. Multiple flash attempts - COM port issues (busy/locked)
10. User restarting computer due to issues

## Next Steps When Resuming
1. Check COM port in Device Manager
2. Close any serial monitors before flashing
3. Flash and test BNO085
4. If still not working:
   - Power cycle BNO085 after connecting PS1
   - Verify TX/RX crossing with multimeter
   - Try I2C mode instead (easier, no PS1 mod needed)

## Alternative: Switch to I2C Mode
If UART continues to fail, can switch to I2C:
- No PS1 modification needed
- BNO085 has I2C address 0x4A (default) or 0x4B
- Already have I2C bus on GPIO 7/8
- Would need code changes to use I2C driver instead of UART

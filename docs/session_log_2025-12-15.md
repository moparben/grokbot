> 📋 **Note:** This is a detailed session log preserved for history.  
> **For current status, see [grokbot-master.md](grokbot-master.md)**

# Grokbot Development Session Log
**Date:** December 14-15, 2025  
**Target:** ESP32-P4 (Waveshare ESP32-P4-NANO)  
**ESP-IDF Version:** v5.5.1  

---

## ⭐ Project Purpose

**Grokbot is a robot companion for the developer's daughter.**

**Her safety is the #1 priority** - more important than any feature or timeline.

This drives key design decisions:
- **Hardware privacy switch** - Physical guarantee that cameras/mic cannot be accessed remotely. No software exploit can bypass a closed switch.
- **Reliability over features** - Better to have fewer features that work safely than many that could fail
- **Quality hardware design** - Careful GPIO selection, proper power management, robust firmware

---

## Session Summary

This session focused on debugging BNO085 IMU sensor integration and resolving GPIO pin conflicts on the Waveshare ESP32-P4-NANO board.

---

## Issues Encountered & Resolved

### 1. Boot Corruption from Strapping Pins
**Problem:** ESP32-P4 boot was corrupted when BNO085 sensor was connected.  
**Root Cause:** Original configuration used GPIO 2/3 which are boot strapping pins.  
**Solution:** Moved UART pins away from strapping pins.

### 2. GPIO Pin Conflicts with Waveshare Board
**Problem:** GPIO 10/11 were assigned for BNO085 UART but are used by the onboard I2S audio codec (ES8311).  
**Discovery:** User noticed their Waveshare board diagram showed different pins than our config.  
**Waveshare ESP32-P4-NANO Reserved Pins:**
- GPIO 7/8: I2C (SDA/SCL) - board defaults
- GPIO 9-13: I2S audio codec (ES8311) - RESERVED, not available
- GPIO 24: USB D+
- GPIO 39-44: SDMMC (TF card)

**Solution:** Updated board_config.h to use:
- UART TX: GPIO 6 (verify availability on headers)
- UART RX: GPIO 20 (verify availability on headers)
- I2C SDA: GPIO 7 (Waveshare default)
- I2C SCL: GPIO 8 (Waveshare default)

### 3. Serial Console Corruption After UART Init
**Problem:** Serial output became garbled after BNO085 UART driver initialized.  
**Possible Causes:**
- GPIO conflicts with other peripherals
- UART driver logging raw binary data
- EYE_LED_GPIO (38) conflicted with BNO085_UART_RX_GPIO

**Temporary Solution:** Disabled FEATURE_BNO085_UART to isolate issue.

### 4. I2C Scan Flooding Logs with Errors
**Problem:** I2C bus scan probes all 127 addresses, logging errors for each non-responding address.  
**Solution:** Disabled FEATURE_I2C_BUS since not currently using I2C devices.

### 5. Preprocessor Directive Mismatch
**Problem:** Build failed with "unterminated #if" error in subsystems.c.  
**Root Cause:** Missing `#endif` for `#if !FEATURE_BNO085_UART && !FEATURE_BNO085_I2C` block.  
**Solution:** Added missing `#endif` after sensor_task function's while loop.

### 6. Flash Connection Issues
**Problem:** esptool reports "serial TX path seems to be down" even though download mode is detected.  
**Possible Causes:**
- Another program has COM5 open
- BNO085 sensor interfering with serial communication
- USB cable issue

**Workaround:** Disconnect BNO085 sensor before flashing.

---

## Current Configuration (board_config.h)

```c
// Feature Flags
#define FEATURE_STATUS_LED      1
#define FEATURE_CONSOLE         1
#define FEATURE_I2C_BUS         0   // Disabled - not using I2C devices
#define FEATURE_SPI_BUS         0
#define FEATURE_MOTORS          0
#define FEATURE_WIFI            0
#define FEATURE_SENSORS         1
#define FEATURE_BNO085_UART     0   // Disabled - debugging console corruption
#define FEATURE_BNO085_I2C      0

// BNO085 UART Pins (for when re-enabled)
#define BNO085_UART_NUM         UART_NUM_1
#define BNO085_UART_TX_GPIO     6       // Verify pin available on headers
#define BNO085_UART_RX_GPIO     20      // Verify pin available on headers
#define BNO085_UART_BAUD        115200

// BNO085 Control Pins
#define BNO085_RST_GPIO         22
#define BNO085_INT_GPIO         -1      // Disabled (was conflicting with USB D+)

// BNO085 I2C Pins (Waveshare defaults)
#define BNO085_I2C_SDA_GPIO     7
#define BNO085_I2C_SCL_GPIO     8
```

---

## Wiring Guide for BNO085 (UART Mode)

Once GPIO pins are verified on your specific Waveshare P4-NANO headers:

| P4-NANO Pin | Direction | BNO085 Pin |
|-------------|-----------|------------|
| GPIO 6      | TX →      | RX (DI)    |
| GPIO 20     | RX ←      | TX (DO)    |
| GPIO 22     | →         | RST        |
| 3.3V        | →         | VIN        |
| GND         | →         | GND        |

**Note:** BNO085 must have PS1 jumper modified (cut GND trace, solder to 3V3) for UART-RVC mode.

---

## Files Modified

1. **firmware/main/board_config.h**
   - Changed BNO085 UART pins from GPIO 10/11 to GPIO 6/20
   - Changed BNO085 I2C pins to GPIO 7/8 (Waveshare defaults)
   - Disabled BNO085_INT_GPIO (was conflicting with USB D+)
   - Disabled FEATURE_BNO085_UART for debugging
   - Disabled FEATURE_I2C_BUS to stop noisy scans

2. **firmware/main/subsystems.c**
   - Fixed unterminated `#if` preprocessor directive
   - Added proper guards for when BNO085 features are disabled

3. **firmware/main/drivers/bno085_uart.h**
   - Updated header documentation with correct Waveshare pins

---

## Build Status

**Last Successful Build:**
- Binary size: 0x38890 bytes (231KB)
- Free space: 97% of partition

**Build Command:**
```powershell
cd D:\esp32_projects\grokbot\firmware
D:\esp32_projects\Espressif\frameworks\esp-idf-v5.5.1\export.ps1
idf.py build
```

**Flash Command:**
```powershell
python -m esptool --chip esp32p4 -p COM5 -b 460800 --before default-reset --after hard-reset write-flash --flash-mode dio --flash-size 16MB --flash-freq 80m 0x2000 build\bootloader\bootloader.bin 0x8000 build\partition_table\partition-table.bin 0x10000 build\grokbot.bin
```

---

## Next Steps

1. **Verify GPIO 6/20 availability** on Waveshare ESP32-P4-NANO pin headers
2. **Find alternative UART pins** if GPIO 6/20 not available (check board schematic)
3. **Re-enable FEATURE_BNO085_UART** once correct pins confirmed
4. **Test clean serial output** with BNO085 disconnected
5. **Wire BNO085** to verified pins and test communication
6. **Debug UART driver** if serial corruption persists

---

## Reference Links

- [Waveshare ESP32-P4-NANO Wiki](https://www.waveshare.com/wiki/ESP32-P4-NANO)
- [Adafruit BNO085 Guide](https://learn.adafruit.com/adafruit-9-dof-orientation-imu-fusion-breakout-bno085)
- [ESP-IDF UART Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32p4/api-reference/peripherals/uart.html)

---

## Hardware Notes

**Waveshare ESP32-P4-NANO Specs:**
- ESP32-P4 chip (dual-core RISC-V, 400MHz)
- 32MB PSRAM
- 16MB Flash
- USB-C connector
- Onboard ES8311 audio codec (uses GPIO 9-13)
- I2C default: SDA=GPIO7, SCL=GPIO8
- 28 programmable GPIOs via headers

**I2C Device Found:**
- Address 0x18: ES8311 audio codec (onboard)

---

## Features Yet To Be Implemented

### 🔴 Not Started

| Feature | Config Flag | GPIO/Pins | Description | Priority |
|---------|-------------|-----------|-------------|----------|
| **WiFi** | `FEATURE_WIFI` | SDIO (GPIO 39-44) | ESP32-C6-MINI module for WiFi 6/BT5 via SDIO | High |
| **Ethernet** | `FEATURE_ETHERNET` | GPIO 27-35 | 100Mbps Ethernet for reliable networking | Medium |
| **Motors** | `FEATURE_MOTORS` | GPIO 4,5,8,9 | PWM motor control for movement | High |
| **Audio (Mic)** | `FEATURE_AUDIO` | GPIO 45,46,47 | I2S microphone for voice input | Medium |
| **Audio (Speaker)** | `FEATURE_AUDIO` | GPIO 48 | Speaker output for voice/sounds | Medium |
| **SD Card** | `FEATURE_SD_CARD` | GPIO 14-19 | TF card for storage/logging | Low |
| **USB OTG** | `FEATURE_USB_OTG` | GPIO 24,25 | USB host/device mode | Low |
| **Eye LEDs** | `FEATURE_EYE_LEDS` | GPIO 38 | WS2812 RGB LEDs for status display | Medium |
| **ToF Sensor** | `FEATURE_TOF_SENSOR` | I2C (0x29) | VL53L1X distance sensor for obstacle avoidance | Medium |
| **Inter-P4 SPI** | `FEATURE_INTER_P4_SPI` | GPIO 10-13 | SPI communication between two P4 boards | Future |
| **Cross Reset** | `FEATURE_CROSS_RESET` | GPIO 21,23 | Watchdog recovery between P4 boards | Future |
| **Privacy Switch** | `FEATURE_PRIVACY_SWITCH` | GPIO 26,27 | Hardware privacy guarantee switch | High |

### 🟡 In Progress / Debugging

| Feature | Config Flag | Status | Notes |
|---------|-------------|--------|-------|
| **BNO085 UART** | `FEATURE_BNO085_UART` | Debugging | Console corruption issue, GPIO conflicts |
| **BNO085 I2C** | `FEATURE_BNO085_I2C` | Disabled | Alternative to UART mode |
| **I2C Bus** | `FEATURE_I2C_BUS` | Disabled | Noisy scan, enable when needed |
| **SPI Bus** | `FEATURE_SPI_BUS` | Disabled | Enable when SPI devices connected |

### 🟢 Working

| Feature | Config Flag | Status |
|---------|-------------|--------|
| **Status LED** | `FEATURE_STATUS_LED` | ✅ Working (GPIO 52) |
| **Console/Logging** | `FEATURE_CONSOLE` | ✅ Working |
| **Heartbeat Task** | - | ✅ Working |
| **NVS Storage** | - | ✅ Working |
| **PSRAM** | - | ✅ 32MB detected |

---

## Feature Implementation Details

### WiFi (FEATURE_WIFI)
**Hardware:** ESP32-C6-MINI module connected via SDIO
**Pins:** GPIO 39-44 (SDIO interface)
**Dependencies:** SDIO driver, C6 module initialization
**Notes:** The C6 module provides WiFi 6 and Bluetooth 5 capabilities

### Ethernet (FEATURE_ETHERNET)
**Hardware:** Onboard 100Mbps Ethernet PHY
**Pins:** 
- MDC: GPIO 31
- MDIO: GPIO 27
- RMII CLK: GPIO 32
- CRS_DV: GPIO 28
- RXD0/1: GPIO 29,30
- TX_EN: GPIO 33
- TXD0/1: GPIO 34,35

### Motors (FEATURE_MOTORS)
**Config:**
- Motor A: GPIO 4 (PWM), GPIO 5 (DIR)
- Motor B: GPIO 8 (PWM), GPIO 9 (DIR)
- PWM Frequency: 25kHz
**Notes:** Requires external motor driver board (L298N, TB6612, etc.)

### Audio (FEATURE_AUDIO)
**Microphone (I2S):**
- SCK: GPIO 45
- WS: GPIO 46
- SD: GPIO 47

**Speaker:**
- Output: GPIO 48 (MX1.25 connector)

**Notes:** Onboard ES8311 audio codec uses GPIO 9-13 (I2S interface)

### Eye LEDs (FEATURE_EYE_LEDS)
**Hardware:** 2x WS2812B/NeoPixel RGB LEDs
**Pin:** GPIO 38
**Use Cases:**
- Blue = Listening
- Green = Ready/OK
- Red = Error
- Purple = Processing
- Rainbow = Startup animation

### BNO085 IMU (FEATURE_BNO085_UART / FEATURE_BNO085_I2C)
**UART Mode (preferred):**
- TX: GPIO 6 (verify on headers)
- RX: GPIO 20 (verify on headers)
- RST: GPIO 22
- Baud: 115200
- Requires PS1 jumper modification on Adafruit board

**I2C Mode:**
- SDA: GPIO 7
- SCL: GPIO 8
- Address: 0x4A
- No hardware modification needed

### Privacy Switch (FEATURE_PRIVACY_SWITCH)
**Purpose:** Hardware guarantee that camera/mic access is impossible when switch is closed
**Pins:**
- Switch Input: GPIO 26 (active LOW)
- Admin Mode LED: GPIO 27

**Behavior:**
- Switch CLOSED (default): Privacy mode, no remote access possible
- Switch OPEN: Admin/service mode for testing

### Inter-P4 Communication (FEATURE_INTER_P4_SPI)
**Purpose:** Communication between two ESP32-P4 boards
**Pins:**
- MOSI: GPIO 11
- MISO: GPIO 12
- SCLK: GPIO 13
- CS: GPIO 10

**Notes:** For future dual-P4 architecture where one handles cameras/web, other handles movement/display

---

## Planned Architecture

```
┌─────────────────────────────────────────────────────────────┐
│                     GROKBOT SYSTEM                          │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐  │
│  │   SENSORS    │    │   MOTION     │    │    COMMS     │  │
│  │              │    │              │    │              │  │
│  │ • BNO085 IMU │    │ • Motor A    │    │ • WiFi (C6)  │  │
│  │ • ToF Sensor │    │ • Motor B    │    │ • Ethernet   │  │
│  │ • Mic        │    │ • Encoders   │    │ • USB OTG    │  │
│  └──────────────┘    └──────────────┘    └──────────────┘  │
│                                                             │
│  ┌──────────────┐    ┌──────────────┐    ┌──────────────┐  │
│  │     UI       │    │   STORAGE    │    │   SAFETY     │  │
│  │              │    │              │    │              │  │
│  │ • Eye LEDs   │    │ • SD Card    │    │ • Privacy SW │  │
│  │ • Speaker    │    │ • NVS        │    │ • Cross-RST  │  │
│  │ • Display*   │    │ • PSRAM      │    │ • Watchdog   │  │
│  └──────────────┘    └──────────────┘    └──────────────┘  │
│                                                             │
│  * Display via MIPI-DSI (future)                           │
└─────────────────────────────────────────────────────────────┘
```

---

## Implementation Priority

### Phase 1: Core (Current)
1. ✅ Basic boot and console
2. ✅ Status LED heartbeat
3. 🔄 BNO085 IMU integration
4. ⬜ Privacy switch

### Phase 2: Movement
1. ⬜ Motor driver integration
2. ⬜ Basic motion commands
3. ⬜ IMU-based stabilization

### Phase 3: Communication
1. ⬜ WiFi initialization (C6 module)
2. ⬜ Basic web server
3. ⬜ Telemetry streaming

### Phase 4: User Interface
1. ⬜ Eye LED animations
2. ⬜ Audio input/output
3. ⬜ Voice commands

### Phase 5: Advanced
1. ⬜ Camera integration (MIPI-CSI)
2. ⬜ Display output (MIPI-DSI)
3. ⬜ Dual-P4 architecture
4. ⬜ AI/ML integration

---

## Environment Info

- **OS:** Windows
- **Python:** 3.13.11
- **ESP-IDF:** v5.5.1
- **Serial Port:** COM5
- **Workspace:** D:\esp32_projects\grokbot

# Grokbot Master Reference Document

> **Last Updated:** December 15, 2025  
> **Status:** Active Development  
> **Target:** ESP32-P4 (Waveshare ESP32-P4-NANO)

---

## ⚠️ CRITICAL: Project Purpose & Safety

**Grokbot is a robot companion being built for the developer's daughter.**

### Safety Priority
**Her safety is the #1 priority - more important than any feature.**

The robot includes a **hardware privacy switch** that provides a physical guarantee:
- When the switch is CLOSED (default): Remote camera/audio/recording access is **IMPOSSIBLE**
- When OPEN (service mode): Full admin access for testing/repair only
- The switch is located inside the robot body, requiring disassembly to access
- **No software can bypass a closed switch** - this is a hardware guarantee

---

## Hardware Platform

### Main Board: Waveshare ESP32-P4-NANO
- **MCU:** ESP32-P4 dual-core RISC-V @ 400MHz
- **Memory:** 32MB PSRAM, 16MB Flash
- **WiFi/BT:** ESP32-C6-MINI module (WiFi 6, BT 5.0) via SDIO
- **Display:** MIPI-DSI interface
- **Camera:** MIPI-CSI interface
- **Audio:** ES8311 codec, onboard microphone, speaker header
- **Connectivity:** Ethernet (100M), USB OTG, TF card slot
- **Wiki:** https://www.waveshare.com/wiki/ESP32-P4-Nano

### GPIO Reservations (DO NOT USE)

| GPIO | Function | Notes |
|------|----------|-------|
| 7, 8 | I2C Bus | Touch panel, external sensors |
| 9-13 | I2S Audio | ES8311 codec (MCLK, SCLK, LRCK, DOUT, DINT) |
| 14-19 | SD Card | TF card slot (if used) |
| 24, 25 | USB OTG | USB D+/D- |
| 27-35, 49-52 | Ethernet | RMII interface |
| 39-44 | SDMMC | ESP32-C6 WiFi module |
| 53 | PA_Ctrl | Audio amplifier control |

### Available GPIO (Current Assignments)
Based on Waveshare pinout and avoiding all conflicts:
- **GPIO 0, 1** - Battery/current ADC (if used)
- **GPIO 2, 3** - ❌ STRAPPING PINS - caused boot corruption
- **GPIO 4, 5** - Motor A PWM/DIR
- **GPIO 6** - **Cross-reset OUT** (to other P4's EN pin)
- **GPIO 20** - Cross-reset monitoring
- **GPIO 21** - BNO085 UART TX (ESP → BNO)
- **GPIO 22** - BNO085 RST
- **GPIO 23** - BNO085 UART RX (BNO → ESP)
- **GPIO 26** - Privacy switch input
- **GPIO 36, 37** - Motor B PWM/DIR
- **GPIO 38** - Eye LEDs (WS2812)

---

## BNO085 IMU Configuration

### Hardware: Adafruit BNO085 Breakout
- **Mode:** UART-RVC (Robot Vacuum Cleaner mode)
- **Baud Rate:** 115200
- **Data Format:** 19-byte packets at ~100Hz

### ⚠️ CRITICAL: I2C Does NOT Work Well with ESP32!
From Adafruit documentation:
> "The BNO08x I2C implementation **violates the I2C protocol** in some circumstances.
> This causes it **not to work well with Espressif ESP32, ESP32-S3**"

**UART-RVC is the correct choice for ESP32-P4!**

### Mode Selection (PS0/PS1 Pins)
| PS1 | PS0 | Mode |
|-----|-----|------|
| Low | Low | I2C (default) |
| Low | **HIGH** | **UART-RVC** ← USE THIS |
| High | Low | UART |
| High | High | SPI |

### To Enable UART-RVC Mode (EASY!)
**Option A:** Pull P0 pin HIGH (connect to 3.3V)
**Option B:** Bridge the P0 solder jumper on back of board

**NO PS1 modification needed for UART-RVC!**

### Wiring (UART-RVC Mode) - From Adafruit Docs
| BNO085 Pin | ESP32-P4 | Notes |
|------------|----------|-------|
| VIN | 3.3V | Power |
| GND | GND | Ground |
| **SDA** | **GPIO 23 (RX)** | **BNO085 TX out** - data to ESP |
| SCL | (not needed) | ESP TX not required for RVC |
| **P0** | **3.3V** | **Pull HIGH for UART-RVC mode!** |
| RST | GPIO 22 (optional) | Hardware reset |

**Key insight:** In UART-RVC mode, the SDA pin becomes TX output!
You only need to receive data - no TX from ESP required.

### GPIO History (UART Attempts)
| GPIO Pair | Result | Issue |
|-----------|--------|-------|
| 2, 3 | ❌ Failed | Strapping pins - boot corruption |
| 10, 11 | ❌ Failed | I2S ES8311 codec conflict |
| 37, 38 | ❌ Failed | Eye LED conflict on GPIO 38 |
| 6, 20 | ❌ Failed | Unknown - still problematic |
| **21, 23** | 🔄 Current | Cross-reset pins repurposed - **TRY THIS** |

### Alternative: I2C Mode (NOT RECOMMENDED for ESP32)
⚠️ **Adafruit warns:** I2C mode has protocol violations that cause issues with ESP32/ESP32-S3.

If you must try I2C:
- Uses existing driver: `firmware/components/drivers/bno085/`
- Example: `examples/imu-test/`
- SDA: GPIO 7, SCL: GPIO 8, Address: 0x4A

**UART-RVC is the better choice for ESP32-P4.**

---

## Planned Features

### Sensors
- [x] BNO085 9-DOF IMU (UART-RVC or I2C)
- [ ] VL53L1X Time-of-Flight distance sensor
- [ ] Battery voltage monitoring (ADC)
- [ ] Current sensing for motors

### Motion System
- [ ] Dual motor drive (H-bridge or BLDC)
- [ ] Encoder feedback for odometry
- [ ] PID motor control
- [ ] Head pan/tilt servos

### Communication
- [ ] WiFi via ESP32-C6 module (SDIO)
- [ ] Bluetooth LE for mobile app
- [ ] Ethernet for development
- [ ] Inter-P4 SPI (for dual-P4 design)

### User Interface
- [ ] MIPI-DSI display for face/expressions
- [ ] RGB "eye" LEDs (WS2812B)
- [ ] Status LED
- [ ] Touch screen

### Audio
- [ ] Microphone input (ES8311)
- [ ] Speaker output
- [ ] Wake word detection
- [ ] Text-to-speech

### Safety & Privacy
- [x] Hardware privacy switch (GPIO 26)
- [x] **Cross-reset watchdog (GPIO 6/20)** - Each P4 can reset the other!
- [ ] Admin mode LED indicator
- [ ] Software watchdog

### Dual-P4 Cross-Reset Watchdog System
Both ESP32-P4 boards monitor each other and can perform a hard reset if one becomes unresponsive.
This is a **safety feature** - ensures the robot can recover from crashes.

**Wiring:**
| P4 #1 | P4 #2 | Function |
|-------|-------|----------|
| GPIO 6 → | EN pin | P4 #1 can reset P4 #2 |
| EN pin ← | GPIO 6 | P4 #2 can reset P4 #1 |

**Protocol:**
1. Each P4 sends heartbeat over Inter-P4 SPI every 500ms
2. If no heartbeat for 3 seconds → assume other P4 is stuck
3. Pulse GPIO 6 LOW for 100ms to hard-reset the unresponsive P4

**CRITICAL:** 10K pullup resistors required on BOTH EN pins!

### Storage
- [ ] TF/SD card support
- [ ] Configuration persistence (NVS)
- [ ] Log storage

### Navigation (Future)
- [ ] SLAM (cameras + ToF)
- [ ] Obstacle avoidance
- [ ] Path planning

---

## Build System

### Environment
- **ESP-IDF:** v5.5.1
- **Location:** `D:\esp32_projects\Espressif\frameworks\esp-idf-v5.5.1`
- **Python venv:** `grokbot.venv`
- **Build targets:** esp32s3, esp32p4

### Build Commands
```powershell
# Activate environment
& D:\esp32_projects\grokbot\grokbot.venv\Scripts\Activate.ps1

# Set target
cd firmware
idf.py set-target esp32p4

# Build
idf.py -B build-esp32p4 build

# Flash
idf.py -B build-esp32p4 -p COM5 flash

# Monitor
idf.py -B build-esp32p4 -p COM5 monitor
```

### VS Code Tasks
Use the pre-configured tasks in `.vscode/tasks.json`:
- `IDF: Build firmware (esp32p4)`
- `IDF: Flash firmware (esp32p4)`
- `IDF: Monitor firmware (esp32p4)`

---

## Current Status (Dec 15, 2025)

### Working
- ✅ Basic firmware builds and runs
- ✅ Status LED heartbeat
- ✅ Console output
- ✅ Boot without strapping pin corruption
- ✅ I2C bus initialization (device at 0x18 detected)

### In Progress
- 🔄 BNO085 UART wiring (GPIO 6/20 needs hardware test)
- 🔄 Finding conflict-free GPIO pair

### Blocked/Pending
- ⏸️ Motors (hardware not connected)
- ⏸️ WiFi (C6 module init needed)
- ⏸️ Display (MIPI-DSI driver)
- ⏸️ Audio (ES8311 driver)

### Known Issues
1. **BNO085 UART GPIO conflicts** - Multiple pairs tried, none working yet
2. **Serial corruption** - UART driver can corrupt console output
3. **Flash "TX path down"** - Intermittent, retry usually works

---

## File Structure

```
grokbot/
├── firmware/
│   ├── main/
│   │   ├── board_config.h      # GPIO and feature definitions
│   │   ├── subsystems.c/.h     # Peripheral initialization
│   │   ├── bno085_uart.c/.h    # IMU UART-RVC driver
│   │   └── main.c              # Entry point
│   ├── boards/
│   │   ├── esp32p4/sdkconfig.defaults
│   │   └── esp32s3/sdkconfig.defaults
│   └── CMakeLists.txt
├── docs/
│   └── grokbot-master.md       # THIS FILE
├── scripts/
│   ├── build_all.ps1
│   ├── flash_all.ps1
│   └── setup_idf_env.ps1
└── grokbot.venv/               # Python virtual environment
```

---

## Key Configuration File

**`firmware/main/board_config.h`** is the central hardware configuration:
- All GPIO assignments
- Feature enable/disable flags
- I2C/SPI/UART parameters
- Safety switch configuration

To enable a feature, change `#define FEATURE_xxx 0` to `1`.

---

## Debug Tips

### Check boot strapping
If board won't boot or behaves erratically:
1. Check GPIO 0, 2, 3 aren't being driven at boot
2. These are strapping pins that configure boot mode

### Serial port issues
- Try COM5 or COM7
- Use 115200 baud
- If "TX path down" error, unplug and retry

### I2C device scan
When `FEATURE_I2C_BUS=1`, the firmware scans for devices at boot.
Known addresses:
- 0x18: Unknown (possibly onboard sensor)
- 0x4A: BNO085 (when connected via I2C)

---

## Session Notes

### December 15, 2025
- Moved BNO085 from GPIO 2/3 (strapping) to multiple alternatives
- Discovered GPIO 10/11 conflict with ES8311 audio codec
- Discovered GPIO 37/38 conflict with Eye LED
- Currently testing GPIO 6/20
- Fixed unterminated `#if` preprocessor directive in subsystems.c
- Created persistent memory for session continuity
- **Critical context:** Robot is for developer's daughter, safety is #1

---

*This document consolidates information from multiple session logs and project files. It is the authoritative reference for the Grokbot project.*

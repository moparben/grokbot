# Grokbot AI Onboarding Guide

> **Read this file at the start of every new chat session.**  
> It provides context for GitHub Copilot (or any AI assistant) to continue work effectively.

---

## ⭐ CRITICAL CONTEXT - READ FIRST

### Project Purpose
**Grokbot is a robot companion being built for the developer's daughter.**

**Her safety is the #1 priority** - more important than any feature.

This context matters because:
- The **hardware privacy switch** is a non-negotiable safety feature
- Quality and reliability matter deeply - this isn't a toy project
- Every design decision should consider child safety
- The name "Grok" means understanding/companion

---

## Project Overview

| Aspect | Details |
|--------|---------|
| **Project** | Grokbot - Robot companion |
| **Location** | `D:\esp32_projects\grokbot` |
| **Hardware** | Waveshare ESP32-P4-NANO |
| **Framework** | ESP-IDF v5.5.1 |
| **MCU** | ESP32-P4 (Dual-core RISC-V @ 400MHz) |
| **Memory** | 32MB PSRAM, 16MB Flash |
| **Serial Port** | COM5 or COM7 |

---

## Key Documentation

| Document | Purpose |
|----------|---------|
| [docs/grokbot-master.md](docs/grokbot-master.md) | **Master reference** - GPIO, features, status |
| [firmware/main/board_config.h](firmware/main/board_config.h) | Pin definitions and feature flags |
| [examples/imu-test/](examples/imu-test/) | BNO085 I2C mode example |
| [firmware/components/drivers/bno085/](firmware/components/drivers/bno085/) | BNO085 I2C driver component |

---

## Current State (Dec 15, 2025)

### What's Working
- ✅ Basic firmware boots and runs
- ✅ Status LED heartbeat (GPIO 52)
- ✅ Console output clean
- ✅ I2C bus initialized (device at 0x18 detected)
- ✅ BNO085 I2C driver component exists

### Active Problem: BNO085 IMU Integration

**Situation:** The BNO085 9-DOF IMU needs to be integrated for orientation sensing.

### ⚠️ CRITICAL DISCOVERY (Dec 15, 2025)
**Adafruit says I2C does NOT work well with ESP32!**
> "The BNO08x I2C implementation violates the I2C protocol in some circumstances.
> This causes it not to work well with Espressif ESP32, ESP32-S3"

**UART-RVC is the correct mode for ESP32-P4!**

### UART-RVC Mode Setup (SIMPLER than we thought!)
To enable UART-RVC mode:
1. Pull **P0 pin HIGH** (connect to 3.3V) - OR bridge P0 solder jumper on back
2. **NO PS1 modification needed!**

Wiring:
- VIN → 3.3V
- GND → GND
- **SDA pin** → ESP32 RX (GPIO 23) - This is BNO085's TX output in RVC mode!
- **P0** → 3.3V (to select UART-RVC mode)
- RST → GPIO 22 (optional)

### GPIO Conflict History
| GPIO Pair | Result | Issue |
|-----------|--------|-------|
| 2, 3 | ❌ Failed | Boot strapping pins |
| 10, 11 | ❌ Failed | ES8311 I2S codec |
| 37, 38 | ❌ Failed | Eye LED conflict |
| 6, 20 | ❌ Failed | Unknown |
| **21, 23** | 🔄 Current | Needs hardware test |

---

## Reserved GPIOs (DO NOT USE)

```
7, 8         - I2C bus (touch, sensors)
9-13         - I2S ES8311 audio codec
14-19        - SD card (if used)
24, 25       - USB OTG
27-35, 49-52 - Ethernet RMII
39-44        - SDMMC WiFi C6 module
53           - PA_Ctrl audio amp
```

---

## Build Commands

```powershell
# Build for ESP32-P4
cd D:\esp32_projects\grokbot\firmware
idf.py -B build-esp32p4 build

# Flash
idf.py -B build-esp32p4 -p COM5 flash

# Monitor
idf.py -B build-esp32p4 -p COM5 monitor
```

Or use VS Code tasks: `IDF: Build firmware (esp32p4)`

---

## Planned Features (Priority Order)

### Safety (CRITICAL)
- [x] Hardware privacy switch defined (GPIO 26)
- [ ] Privacy switch implementation
- [ ] Cross-reset watchdog
- [ ] E-stop integration

### Sensors
- [ ] **BNO085 IMU** ← Current focus
- [ ] VL53L1X ToF distance
- [ ] Battery monitoring

### Motion
- [ ] Motor control (GPIO 4/5, 36/37)
- [ ] Encoder feedback
- [ ] PID control

### UI
- [ ] Eye LEDs (WS2812B, GPIO 38)
- [ ] Audio (ES8311)
- [ ] Display (MIPI-DSI)

---

## Debugging Tips

1. **If boot fails:** Check GPIO 0, 2, 3 aren't driven at boot (strapping pins)
2. **Serial corruption:** UART driver can interfere with console - disable BNO085 UART
3. **Flash "TX path down":** Retry, usually works second time
4. **I2C not responding:** Check `FEATURE_I2C_BUS=1` in board_config.h

---

## Recommended Next Steps

1. **Try I2C mode for BNO085** - Use existing component and imu-test example
2. **Wire BNO085:** VIN→3.3V, GND→GND, SDA→GPIO7, SCL→GPIO8
3. **Test with imu-test example** before integrating into main firmware
4. **Once working:** Enable in board_config.h with `FEATURE_BNO085_I2C=1`

---

## Memory Persistence

AI memory is stored at: `/memories/grokbot-project.md`

Use this to recall context between sessions. Update it when significant progress is made.

---

## Session Continuity

When starting a new chat:
1. Read this file (`AI_ONBOARDING.md`)
2. Check `/memories/grokbot-project.md` for latest status
3. Review [grokbot-master.md](docs/grokbot-master.md) for technical details
4. Ask user for any updates since last session

---

*Last updated: December 15, 2025*

> ⚠️ **DEPRECATED:** This document contains outdated information.  
> **Please refer to [grokbot-master.md](grokbot-master.md) for current project status.**

# Grokbot ESP32-P4 Robot Project Summary

## Project Overview
A robot project using the **Waveshare ESP32-P4-Nano** development board with various sensors and motor control capabilities.

---

## Hardware Configuration

### Main Board
- **MCU**: Waveshare ESP32-P4-Nano
- **USB Port**: COM5
- **Framework**: ESP-IDF 5.5.1

### IMU Sensor - BNO085
- **Mode**: UART (RVC mode at 115200 baud)
- **Wiring**:
  - Red wire → 3.3V (VCC)
  - Black wire → GND
  - Yellow wire → GPIO 37 (TX to BNO085 RX)
  - Blue wire → GPIO 38 (RX from BNO085 TX)
- **Note**: GPIO 37/38 are the correct UART0 pins per Waveshare wiki

### Development Environment
- **ESP-IDF**: 5.5.1 at `D:\esp32_projects\Espressif\frameworks\esp-idf-v5.5.1`
- **Python**: 3.13.11 at `D:\esp32_projects\Espressif\python_env\idf5.5_py3.13_env`
- **Workspace**: `D:\esp32_projects\grokbot`

---

## Current Project Status (December 15, 2025)

### What's Working
- ✅ Firmware compiles successfully (with GPIO 2/3 config - old)
- ✅ Firmware flashes to ESP32-P4
- ✅ BNO085 UART driver initializes (sees "BNO085 UART initialized" in logs)
- ✅ Motor control infrastructure in place
- ✅ Project structure and build system configured

### Current Blocker
- ❌ **CMake crashes during build** with exit code 3221225786 (0xC0000374 = STATUS_HEAP_CORRUPTION)
- This prevented rebuilding firmware after updating GPIO pins from 2/3 to 37/38

### Root Cause Found
- Old ESP-IDF installation on C: drive was conflicting with D: drive installation
- Python path was pointing to `C:\esp32_projects\Espressif\python_env\idf5.4_py3.13_env`
- **Solution Applied**: Deleted entire `C:\esp32_projects` folder
- **Next Step**: Restart laptop and attempt fresh build

### BNO085 Issue History
1. Initially configured with GPIO 2/3
2. Build succeeded, firmware flashed
3. BNO085 UART initialized but showed "No data" errors
4. Research showed Waveshare ESP32-P4-Nano uses GPIO 37/38 for UART0
5. Updated `board_config.h` to use GPIO 37/38
6. CMake started crashing before rebuild could complete

---

## Key Configuration Files

### firmware/main/board_config.h
```c
// BNO085 IMU Configuration (UART mode)
#define BNO085_UART_NUM         UART_NUM_0
#define BNO085_UART_TX_GPIO     37    // Yellow wire to BNO085 RX
#define BNO085_UART_RX_GPIO     38    // Blue wire to BNO085 TX  
#define BNO085_UART_BAUD_RATE   115200

// Feature flags
#define FEATURE_SENSORS         1
#define FEATURE_BNO085_UART     1
```

### firmware/sdkconfig (key settings)
- Target: ESP32-P4
- Flash size: 16MB
- PSRAM enabled

---

## Project Structure
```
D:\esp32_projects\grokbot\
├── firmware/
│   ├── main/
│   │   ├── app_main.c          # Main application entry
│   │   ├── board_config.h      # Hardware pin definitions
│   │   ├── bno085_uart.c/.h    # BNO085 UART driver
│   │   ├── sensor_hub.c/.h     # Sensor management
│   │   ├── motor_control.c/.h  # Motor driver
│   │   └── CMakeLists.txt
│   ├── components/             # ESP-IDF components
│   ├── build/                  # Build output (delete for clean build)
│   ├── sdkconfig               # Project configuration
│   └── CMakeLists.txt
├── docs/                       # Documentation
└── grockbot.venv/              # Python virtual environment
```

---

## Future Features (Not Yet Implemented)

### Sensors
- [ ] Additional IMU modes (I2C, SPI for BNO085)
- [ ] Ultrasonic distance sensors
- [ ] IR proximity sensors
- [ ] Camera integration
- [ ] GPS module

### Motor Control
- [ ] PID control for motors
- [ ] Encoder feedback
- [ ] Speed ramping/acceleration curves
- [ ] Differential drive kinematics

### Navigation
- [ ] Odometry calculation
- [ ] Path planning algorithms
- [ ] Obstacle avoidance
- [ ] SLAM (Simultaneous Localization and Mapping)

### Communication
- [ ] Wi-Fi remote control
- [ ] Bluetooth control
- [ ] Web dashboard
- [ ] MQTT telemetry

### Power Management
- [ ] Battery monitoring
- [ ] Low power modes
- [ ] Charging status

---

## Troubleshooting Notes

### CMake Crash (0xC0000374)
- **Symptom**: CMake crashes immediately after "Found Git" message
- **Error Code**: 3221225786 (0xC0000374 = Windows heap corruption)
- **Causes Found**:
  1. Conflicting ESP-IDF installations (C: vs D: drive)
  2. Too many VS Code terminals open (30+)
  3. File locks from previous failed builds
- **Solutions**:
  1. Delete old ESP-IDF on C: drive ✅ Done
  2. Restart laptop to clear memory/file locks
  3. Delete build folder before rebuilding
  4. Build from fresh terminal with clean environment

### BNO085 "No Data" 
- Check wiring: TX→RX crossed correctly
- Verify GPIO pins match board's actual UART pins
- BNO085 needs time to initialize after power-up
- Try GPIO 37/38 (Waveshare ESP32-P4-Nano UART0 pins)

---

## Next Session Action Items

1. **Restart laptop** (clears memory, file locks, environment)
2. **Open VS Code fresh** with grokbot project
3. **Delete build folder**: `D:\esp32_projects\grokbot\firmware\build`
4. **Build firmware**: `idf.py build`
5. **Flash firmware**: `idf.py -p COM5 flash`
6. **Monitor output**: `idf.py -p COM5 monitor`
7. **Verify BNO085** receives data on GPIO 37/38

---

## Session Log Summary (December 14-15, 2025)

### Session Timeline
1. Started with BNO085 "no data" issue on GPIO 2/3
2. Researched Waveshare wiki - found correct pins are GPIO 37/38
3. Updated board_config.h with new GPIO pins
4. Attempted rebuild - CMake crashed with heap corruption
5. Tried multiple fixes:
   - Different CMake versions (3.30.2, 4.2.0)
   - Windows Defender exclusions
   - Environment variable fixes
6. Discovered C: drive had old ESP-IDF conflicting with D: drive
7. Deleted C:\esp32_projects folder
8. User restarting laptop for fresh start

### Key Discovery
The CMake crash was likely caused by **mixed Python environments** between two ESP-IDF installations on different drives. The `export.ps1` script was loading the correct IDF path but Python was importing from the wrong (C: drive) environment.

---

*Last Updated: December 15, 2025*

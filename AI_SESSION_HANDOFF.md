# 🤖 Grokbot AI Session Handoff Document
> **Created:** December 16, 2025  
> **Last Updated:** December 2025 - Eye Display Implementation
> **Purpose:** Everything a new AI assistant needs to continue this project seamlessly

---

## 🎯 MISSION CRITICAL - READ FIRST

**The user is building a robot for their young daughter. SAFETY IS PRIORITY #1.**

This is the **Grokbot** project - a personal robot using dual ESP32-P4 microcontrollers with:
- BNO085 IMU for motion sensing (fall detection, orientation)
- Animated eye displays for expressions
- Cross-reset watchdog for safety (if one board crashes, the other recovers it)

---

## 📍 Current Status: EYE DISPLAY IMPLEMENTATION COMPLETE ✅

### Latest Accomplishment (December 2025)
**Dual animated eye displays implemented!** Using Waveshare 0.71" round LCD modules:
- ✅ GC9D01 driver written for 160x160 round displays
- ✅ Procedural eye rendering (no pre-rendered assets needed)
- ✅ Animation engine with auto-blink and eye tracking
- ✅ 12 emotional states: IDLE, HAPPY, SAD, ANGRY, SLEEPY, LOVE, etc.
- ✅ SLEEPY state for low battery indication
- ✅ ERROR state shows X eyes
- ✅ Dual SPI displays with shared bus, separate CS

### Display Hardware
- **Model:** Waveshare 0.71" Round LCD Module
- **Controller:** GC9D01
- **Resolution:** 160x160 RGB565
- **Interface:** SPI @ 40MHz
- **Wiki:** https://www.waveshare.com/wiki/0.71inch_LCD_Module

### Previous Accomplishments
- ES8311 Audio Driver fully working
- BNO085 IMU in UART-RVC mode

### Build System Status
CMake build issue was resolved by using the working ESP-IDF environment. 
- **Serial Port:** COM7 (JTAG/USB bridge on ESP32-P4-NANO)
- **Build tested with:** `idf.py -p COM7 flash monitor`

### Backup Location
`D:\esp32_projects\grokbot_backup_2025-12-16_002013`

---

## 🔧 Hardware Configuration

### Target Platform
- **Board:** Waveshare ESP32-P4-NANO (dual boards planned)
- **Architecture:** RISC-V (not Xtensa like ESP32-S3)
- **Toolchain:** `riscv32-esp-elf` at `D:\esp32_projects\Espressif\tools\riscv32-esp-elf\esp-14.2.0_20241119`

### BNO085 IMU Wiring (UART-RVC Mode)
| BNO085 Pin | ESP32-P4 Pin | Notes |
|------------|--------------|-------|
| VIN | 3.3V | Also connect P0 to 3.3V for UART-RVC mode |
| GND | GND | |
| SDA | GPIO 23 | UART RX on ESP32, TX from BNO085 |
| P0 | 3.3V | **CRITICAL: Enables UART-RVC mode** |

- **Baud rate:** 115200
- **Mode:** UART-RVC (simplest, heading+accel only, no config needed)
- **Data format:** 19-byte packets starting with 0xAA 0xAA

### Cross-Reset Watchdog (Dual P4 Architecture)
- **Board A GPIO 6** ↔ **Board B GPIO 20** (directly connected)
- **Board A GPIO 20** ↔ **Board B GPIO 6** (directly connected)
- Each board monitors heartbeat from the other
- If heartbeat stops for 5 seconds, trigger reset via GPIO

### Eye Displays (Waveshare 0.71" Round LCD)
| Function | GPIO | Notes |
|----------|------|-------|
| SPI MOSI | 37 | Shared data line |
| SPI SCLK | 36 | Shared clock |
| SPI DC | 5 | Data/Command select (shared) |
| SPI RST | 6 | Reset (shared, also cross-reset!) |
| SPI BL | 48 | Backlight PWM (shared) |
| Left CS | 4 | Left eye chip select |
| Right CS | 47 | Right eye chip select |

**Key Files:**
- `components/display_eyes/src/gc9d01_driver.c` - Low-level LCD driver
- `components/display_eyes/src/eye_renderer.c` - Animation engine
- `components/display_eyes/src/eye_data.h` - Emotion parameters

**Emotional States:**
- `EYE_STATE_SLEEPY` - For low battery (half-closed eyes)
- `EYE_STATE_LISTENING` - Wake word detected (dilated pupils)
- `EYE_STATE_ERROR` - X-shaped eyes
- `EYE_STATE_LOVE` - Heart-shaped iris

### ES8311 Audio Codec (VERIFIED WORKING)
| Function | GPIO | Notes |
|----------|------|-------|
| I2C SDA | 7 | Shared I2C bus @ 400kHz |
| I2C SCL | 8 | ES8311 addr: 0x18 |
| I2S MCLK | 13 | 6.144MHz (384x @ 16kHz) |
| I2S SCLK | 12 | Bit clock |
| I2S LRCK | 10 | Word select |
| I2S DIN | 9 | Audio data in (for recording) |
| I2S DOUT | 11 | Audio data out (for playback) |
| PA Enable | 53 | NS4150B amp - active HIGH |

**Key Configuration:**
- Sample rate: 16kHz
- Bit depth: 16-bit stereo
- MCLK multiplier: 384x (CRITICAL - 256x does NOT work!)
- Volume: 0-100 maps to register 0x00-0xFF

---

## 📁 Project Structure

```
D:\esp32_projects\grokbot\
├── firmware/                    # Main ESP-IDF project
│   ├── main/
│   │   ├── main.c              # Entry point
│   │   ├── subsystems.c/.h     # Subsystem initialization
│   │   ├── board_config.h      # Pin definitions (UPDATED with audio)
│   │   ├── dual_p4_architecture.h
│   │   └── drivers/
│   │       ├── bno085_rvc.c/.h # BNO085 UART-RVC driver ✅ DONE
│   │       ├── cross_reset.c/.h # Watchdog driver ✅ DONE
│   │       └── eye_leds.c/.h   # LED expressions ✅ DONE
│   ├── components/
│   │   └── drivers/
│   │       └── audio/          # ES8311 audio driver ✅ NEW
│   │           ├── audio_driver.c
│   │           ├── include/audio_driver.h
│   │           └── CMakeLists.txt
│   ├── boards/
│   │   ├── esp32p4/sdkconfig.defaults
│   │   └── esp32s3/sdkconfig.defaults
│   └── CMakeLists.txt
├── examples/
│   └── audio-test/             # Standalone audio test project ✅ VERIFIED WORKING
│       └── main/audio_test_main.c
├── scripts/
│   ├── debug_build.ps1         # Build with logging
│   ├── flash_all.ps1           # Flash firmware
│   └── setup_idf_env.ps1       # Environment setup
├── docs/
│   ├── grokbot-master.md       # Master documentation
│   └── hardware.md             # Hardware details
└── .vscode/
    └── tasks.json              # VS Code build tasks
```

---

## 🔑 Key Code Files

### BNO085 UART-RVC Driver
**File:** `firmware/main/drivers/bno085_rvc.c`

The driver:
1. Configures UART1 at 115200 baud on GPIO 23
2. Reads 19-byte packets starting with 0xAA 0xAA
3. Parses heading (yaw), pitch, roll, and accelerometer data
4. Provides `bno085_rvc_init()` and `bno085_rvc_get_data()`

**Data structure:**
```c
typedef struct {
    float yaw;      // Heading in degrees (0-360)
    float pitch;    // Pitch in degrees
    float roll;     // Roll in degrees  
    float accel_x;  // Acceleration in m/s²
    float accel_y;
    float accel_z;
} bno085_rvc_data_t;
```

### Cross-Reset Watchdog
**File:** `firmware/main/drivers/cross_reset.c`

- Uses GPIO 6 (output heartbeat) and GPIO 20 (input monitor)
- FreeRTOS task sends heartbeat every 500ms
- If no heartbeat received for 5 seconds, resets partner board

### Pin Configuration
**File:** `firmware/main/board_config.h`

```c
// BNO085 UART-RVC
#define BNO085_UART_NUM     UART_NUM_1
#define BNO085_UART_TX_PIN  GPIO_NUM_22  // Not used in RVC mode
#define BNO085_UART_RX_PIN  GPIO_NUM_23  // Receives data from BNO085

// Cross-reset watchdog
#define CROSS_RESET_OUTPUT_PIN  GPIO_NUM_6
#define CROSS_RESET_INPUT_PIN   GPIO_NUM_20

// Eye LEDs
#define EYE_LEFT_PIN   GPIO_NUM_4
#define EYE_RIGHT_PIN  GPIO_NUM_5
```

### ES8311 Audio Driver ✅ NEW
**Files:** `firmware/components/drivers/audio/audio_driver.c` and `.h`

The driver:
1. Initializes I2C bus with new Master Bus API (not legacy driver)
2. Configures ES8311 codec with verified register settings
3. Sets up I2S at 16kHz with 6.144MHz MCLK
4. Provides tone generation, melodies, and WAV playback

**API:**
```c
esp_err_t audio_init(void);             // Must call first
void audio_pa_enable(bool enable);      // Control amplifier
esp_err_t audio_set_volume(uint8_t vol);// 0-100%
esp_err_t audio_play_tone(uint16_t hz, uint16_t ms, uint8_t vol);
esp_err_t audio_play_startup_sound(void);
esp_err_t audio_play_happy_sound(void);
esp_err_t audio_play_error_sound(void);
```

**Critical Lessons Learned:**
1. MCLK MUST be 384x (6.144MHz), NOT 256x - codec won't work otherwise
2. Reset sequence: Write 0x1F to REG00, wait, then write 0x80
3. Volume maps 0-100 to 0x00-0xFF for consistent loudness
4. PA GPIO53 must be HIGH for sound output (NS4150B is active HIGH)

---

## 🛠️ Environment Details (Before Reinstall)

### Current Installation (HAS ISSUES)
- **ESP-IDF:** v5.5.1 at `D:\esp32_projects\Espressif\frameworks\esp-idf-v5.5.1`
- **Tools:** `D:\esp32_projects\Espressif\tools\`
- **Python venv:** `D:\esp32_projects\Espressif\python_env\idf5.5_py3.13_env` (Python 3.13.11)
- **Project venv:** `D:\esp32_projects\grokbot\grokbot.venv` (Python 3.13.11)

### Known Environment Issues
1. **Python 3.13** - Too new, causes cmake crashes
2. **Long paths** - `D:\esp32_projects\Espressif\...` may cause issues
3. **VS Code TEMP override** - Sets `TEMP=D:\VSCodeTemp` which can cause problems
4. **System Python conflict** - `C:\Users\mopar\AppData\Local\Programs\Python\Python313` mixes with ESP-IDF

### After Reinstall - Recommended Paths
```
C:\esp-idf\                     # ESP-IDF framework
C:\esp-tools\                   # Toolchains, cmake, ninja
C:\esp-idf\python_env\          # Python virtual environment
D:\esp32_projects\grokbot\      # Project stays here (backed up)
```

---

## 📋 Build Commands (After Fix)

### Set Target
```powershell
cd D:\esp32_projects\grokbot\firmware
idf.py set-target esp32p4
```

### Build
```powershell
idf.py -B build-esp32p4 -DSDKCONFIG_DEFAULTS=boards/esp32p4/sdkconfig.defaults build
```

### Flash
```powershell
idf.py -B build-esp32p4 -p COM5 flash
```

### Monitor
```powershell
idf.py -B build-esp32p4 -p COM5 monitor
```

---

## 🎨 Eye LED Expressions (Planned)

| Expression | Pattern |
|------------|---------|
| Happy | Both eyes bright white, slight pulse |
| Curious | One eye brighter, slight tilt animation |
| Sleepy | Dim, slow fade in/out |
| Alert | Bright red flash |
| Love | Pink/red hearts pattern |

---

## 🧪 Testing Checklist (After Build Works)

- [ ] BNO085 outputs valid heading data
- [ ] Accelerometer readings are reasonable (9.8 m/s² when still)
- [ ] Cross-reset triggers when partner board disconnected
- [ ] Eye LEDs respond to commands
- [ ] Fall detection triggers on sharp acceleration change

---

## 💡 Tips for New AI Assistant

1. **Always prioritize safety** - This robot will be around a child
2. **Build system works** - Audio test project builds/flashes successfully on COM7
3. **User prefers PowerShell** - All scripts are .ps1
4. **User has excellent hardware knowledge** - Does wiring, 3D printing, mechanical design
5. **Keep explanations concise** - User appreciates efficiency
6. **ES8311 audio driver is complete** - Verified working, component ready
7. **XIAO handles WiFi/BT** - ESP32-C6 on P4 was unreliable, using S3 instead
8. **Display is MIPI** - Small 2.4"/2.8" on chest, NOT large 10.1" panel

---

## 📞 Quick Reference

| Item | Value |
|------|-------|
| Target | ESP32-P4 (RISC-V) |
| ESP-IDF Version | v5.5.1 (recommend v5.4.1 after reinstall) |
| Python | 3.13.11 (recommend 3.11 after reinstall) |
| IMU | BNO085 in UART-RVC mode |
| IMU Baud | 115200 |
| IMU RX Pin | GPIO 23 |
| Serial Ports | COM5 (P4 UART), COM7 (P4 JTAG) |
| Backup | `D:\esp32_projects\grokbot_backup_2025-12-16_002013` |

---

## 🚀 Next Steps

1. ~~ES8311 audio driver~~ ✅ DONE
2. Finish pan/tilt mechanical assembly
3. Test XIAO ESP32-S3 with ESP-SR wake word
4. Implement BT A2DP sink for iPhone audio passthrough
5. Define ribbon cable protocol (Head P4 ↔ Body P4)
6. Integrate display driver (MIPI, small panel)
7. Servo control for 6 motors
8. Safety watchdog between P4s

---

*Good luck! Audio is working, architecture is solid, just need the mechanical bits sorted.* 🤖❤️

---

## 🏗️ Physical Architecture (Updated December 16, 2025)

### Overview
Grokbot uses **4 microcontrollers** across HEAD and TORSO:

```
┌─────────────────────────────────────────────────────────────────────┐
│                           GROKBOT                                    │
├─────────────────────────────────────────────────────────────────────┤
│                                                                      │
│  ╔═══════════════════════════════════════════════════════════════╗  │
│  ║                         HEAD                                   ║  │
│  ║  ┌─────────────┐                    ┌─────────────┐           ║  │
│  ║  │ XIAO S3     │                    │ XIAO S3     │           ║  │
│  ║  │ LEFT EYE    │                    │ RIGHT EYE   │           ║  │
│  ║  │ • Camera    │                    │ • Camera    │           ║  │
│  ║  │ • Vision Bd │                    │ • Vision Bd │           ║  │
│  ║  │ • PDM Mic   │                    │ • PDM Mic   │           ║  │
│  ║  │ • ESP-SR    │                    │ • WiFi/BT   │           ║  │
│  ║  └──────┬──────┘                    └──────┬──────┘           ║  │
│  ║         │ UART                             │ UART             ║  │
│  ║         └──────────────┬───────────────────┘                  ║  │
│  ║                        ▼                                       ║  │
│  ║              ┌──────────────────┐                             ║  │
│  ║              │  HEAD ESP32-P4   │                             ║  │
│  ║              │  • XIAO coord    │                             ║  │
│  ║              │  • Audio record  │                             ║  │
│  ║              └────────┬─────────┘                             ║  │
│  ╚═══════════════════════╪═══════════════════════════════════════╝  │
│                          │ MG-90 TILT SERVO                         │
│                    ══════╪══════  NECK PAN SERVO                    │
│                          │                                           │
│                 ┌────────┴────────┐                                  │
│                 │  Ribbon Cable   │  (P4 ↔ P4 communication)        │
│                 └────────┬────────┘                                  │
│                          │                                           │
│  ╔═══════════════════════╪═══════════════════════════════════════╗  │
│  ║                    TORSO                                       ║  │
│  ║  ┌─────────────────────────────────────────────────────────┐  ║  │
│  ║  │ BACK CHAMBER                                             │  ║  │
│  ║  │  • BODY ESP32-P4-NANO (main brain)                      │  ║  │
│  ║  │  • BNO085 IMU                                            │  ║  │
│  ║  │  • Audio Amp (for ES8311)                                │  ║  │
│  ║  └─────────────────────────────────────────────────────────┘  ║  │
│  ║  ┌─────────────────────────────────────────────────────────┐  ║  │
│  ║  │ MAIN CHAMBER (tight fit!)                                │  ║  │
│  ║  │  • 4x Servos: 2 arm + 2 hip                             │  ║  │
│  ║  │  • Battery                                               │  ║  │
│  ║  │  • Buck converter                                        │  ║  │
│  ║  │  • 2x Speakers (side-firing)                            │  ║  │
│  ║  └─────────────────────────────────────────────────────────┘  ║  │
│  ║  ┌─────────────────────────────────────────────────────────┐  ║  │
│  ║  │ FRONT CHAMBER                                            │  ║  │
│  ║  │  • 2.4"/2.8" MIPI Display (face/expressions)            │  ║  │
│  ║  │  • Sound grille (pass-through)                          │  ║  │
│  ║  └─────────────────────────────────────────────────────────┘  ║  │
│  ╚════════════════════════════════════════════════════════════════╝  │
│                                                                      │
└─────────────────────────────────────────────────────────────────────┘
```

### MCU Responsibilities

| MCU | Location | Tasks |
|-----|----------|-------|
| **XIAO #1** | Left Eye | Camera, PDM mic, ESP-SR wake word, face detection |
| **XIAO #2** | Right Eye | Camera, PDM mic, **WiFi + BT audio** (iPhone connection) |
| **Head P4** | Head | XIAO coordination, audio recording, ribbon uplink to Body |
| **Body P4** | Back chamber | **Main brain**: Display (MIPI), ES8311 audio, 6 servos, IMU, power |

### Servo Allocation (All controlled by Body P4)

| Servo | Location | Type |
|-------|----------|------|
| Pan | Neck | Standard |
| Tilt | Head | MG-90 mini |
| Arm Left | Main chamber | Standard |
| Arm Right | Main chamber | Standard |
| Hip Left | Main chamber | Standard |
| Hip Right | Main chamber | Standard |

### Communication
- **XIAOs ↔ Head P4:** UART (wake word triggers, camera data)
- **Head P4 ↔ Body P4:** Ribbon cable (protocol TBD - SPI or UART)
- **Head travel:** 180° pan, 180° tilt (no slip ring needed)

### Audio Modes

**Mode 1: Standalone (no phone)**
```
PDM Mic → XIAO ESP-SR → Wake detected → XIAO WiFi → Grok API
                                                        ↓
Body P4 ← Head P4 ← XIAO ← TTS Response
    ↓
ES8311 → Amp → Side Speakers
```

**Mode 2: iPhone Connected (Bluetooth)**
```
iPhone Mic → BT HFP → Right XIAO → Grok API → TTS → BT A2DP → iPhone Speaker
            (bypasses local audio - better quality)
```

### Safety Features
1. **Hardware Privacy Switch** (GPIO 26) - Physical kill switch
2. **Cross-reset Watchdog** - GPIO 6 ↔ GPIO 20 between P4s
3. **Local wake word** - No cloud until "Hey Grokbot" detected
4. **Parental controls** - Web interface for content filtering

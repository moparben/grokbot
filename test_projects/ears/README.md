# Ears - Microphone Array Project

Robot audio input system for sound source localization and voice detection.

## Goal

Detect when someone is speaking and determine their direction so the robot can turn its head to look at them. Integrate with STT (speech-to-text) pipeline.

## Hardware

### Microphone
- **Model**: INMP441
- **Type**: Digital I2S MEMS Microphone
- **Pattern**: Omnidirectional
- **Interface**: I2S (digital, no ADC needed!)
- **Supply**: 3.3V
- **SNR**: 61 dB
- **Sensitivity**: -26 dBFS

### INMP441 Pinout
| Pin | Function |
|-----|----------|
| VDD | 3.3V power |
| GND | Ground |
| SD | Serial Data (I2S) |
| WS | Word Select / LRCLK |
| SCK | Serial Clock / BCLK |
| L/R | Channel select (LOW=Left, HIGH=Right) |

### Sound Localization Setup
For direction detection, using **2 INMP441 microphones** (stereo pair):
- Left mic: L/R pin → GND
- Right mic: L/R pin → VDD
- Both share same I2S bus (SCK, WS)
- Each outputs on SD when its channel is active
- **Mic spacing: 57mm** (robot ear positions)

```
    [LEFT MIC]              [RIGHT MIC]
        │                       │
       I2S                     I2S  
        │       ◄── 57mm ──►    │
        +───────► ESP32-P4 ◄────+
                   I2S RX
```

### ESP32-P4 I2S Pins (in robot head)
The ESP32-P4 has dedicated LP (low-power) I2S for always-on audio and HP (high-performance) I2S.

| Signal | GPIO (suggested) | Notes |
|--------|------------------|-------|
| BCLK (SCK) | TBD | Bit clock |
| WS (LRCLK) | TBD | Word select |
| DIN (SD) | TBD | Data from mics |

**ESP32-P4 Audio Advantages:**
- Dual-core 400MHz RISC-V (vs 240MHz on S3)
- Hardware audio codec support
- More RAM for audio buffers
- Better DSP performance for cross-correlation

## Sound Source Localization

### Method: Interaural Time Difference (ITD)
Sound arrives at the closer mic first. By measuring the time delay between left and right channels, we can estimate direction.

- Mic spacing: ~10-15cm recommended
- Max delay at 90°: ~0.4ms (for 14cm spacing)
- Sample rate: 16kHz+ needed for good resolution

### Method: Interaural Level Difference (ILD)
Sound is louder at the closer mic. Simpler but less accurate than ITD.

### Algorithm Overview
1. Capture stereo audio buffer
2. Cross-correlate left and right channels
3. Find peak correlation lag
4. Convert lag to angle estimate
5. Filter/smooth over time
6. Output direction for head tracking

## Project Goals

- [x] Define hardware (INMP441 x2)
- [ ] Basic I2S audio capture on ESP32-S3
- [ ] Stereo capture (left + right mics)
- [ ] Sound level / voice activity detection (VAD)
- [ ] Sound direction estimation (left/center/right)
- [ ] Angle output for head servo control
- [ ] Integration with eyes (look toward sound)
- [ ] STT pipeline integration

## Software Architecture

```
┌─────────────┐     ┌─────────────┐     ┌─────────────┐
│  INMP441 L  │────▶│             │     │   Head      │
│  INMP441 R  │────▶│   ESP32-S3  │────▶│   Servo     │
└─────────────┘     │             │     │   Control   │
                    │  - I2S RX   │     └─────────────┘
                    │  - DSP      │
                    │  - VAD      │     ┌─────────────┐
                    │  - Angle    │────▶│   Eyes      │
                    │    Estimate │     │   (look at) │
                    └─────────────┘     └─────────────┘
                          │
                          ▼
                    ┌─────────────┐
                    │   STT       │
                    │  (external) │
                    └─────────────┘
```

## Resources

- [ESP-IDF I2S Driver](https://docs.espressif.com/projects/esp-idf/en/latest/esp32s3/api-reference/peripherals/i2s.html)
- INMP441 Datasheet
- Sound localization algorithms

## Folder Structure

```
ears/
├── README.md
├── hardware/          # Schematics, wiring diagrams
├── firmware/          # ESP-IDF projects
│   ├── 01_i2s_capture/    # Basic mic test
│   ├── 02_stereo/         # Dual mic capture
│   └── 03_localization/   # Direction detection
└── docs/              # Documentation, datasheets
```

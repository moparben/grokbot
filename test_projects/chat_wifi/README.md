# Jarvis - AI Voice Assistant

Local AI chatbot using ESP32-P4 with stereo INMP441 mics and Grok API.

## Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        ESP32-P4 Nano                            │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐  │
│  │ INMP441  │───▶│  Audio   │───▶│  WiFi    │───▶│  HTTP    │  │
│  │ Stereo   │    │ Capture  │    │  Stack   │    │ Client   │  │
│  └──────────┘    └──────────┘    └──────────┘    └──────────┘  │
└─────────────────────────────────────────────────────────────────┘
                              │
                              ▼ WebSocket/HTTP
┌─────────────────────────────────────────────────────────────────┐
│                     Host PC (Python)                            │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐    ┌──────────┐  │
│  │ Audio    │───▶│ Whisper  │───▶│ Grok     │───▶│  TTS     │  │
│  │ Receive  │    │ STT      │    │ API      │    │ (Piper)  │  │
│  └──────────┘    └──────────┘    └──────────┘    └──────────┘  │
└─────────────────────────────────────────────────────────────────┘
```

## Components

### ESP32-P4 Firmware (`firmware/`)
- Stereo I2S capture from dual INMP441 mics
- WiFi connectivity
- WebSocket client to stream audio to host
- Audio playback (I2S DAC or PDM speaker)
- Wake word detection (optional, on-device)

### Host Service (`host/`)
- Python WebSocket server
- Speech-to-Text (Whisper local or API)
- Grok API integration for chat responses
- Text-to-Speech (Piper local or cloud TTS)

## Hardware

| Component | Connection |
|-----------|------------|
| INMP441 Left | L/R→GND, BCLK→GPIO4, WS→GPIO5, SD→GPIO6 |
| INMP441 Right | L/R→3.3V, BCLK→GPIO4, WS→GPIO5, SD→GPIO6 |
| Speaker/DAC | TBD (I2S or PDM output) |

## Setup

### 1. Configure WiFi and API keys
```bash
cp host/.env.example host/.env
# Edit host/.env with your Grok API key and WiFi credentials
```

### 2. Build and flash ESP32-P4 firmware
```bash
cd firmware
idf.py build flash monitor
```

### 3. Start host service
```bash
cd host
pip install -r requirements.txt
python server.py
```

## API Keys Required

- **Grok API**: Get from https://console.x.ai/

# ESP32 Digital Sticky Note Display

A wireless display system for ESP32 CYD that shows remotely-updated text, designed for live worship environments.

## Project Overview

**Use Case:** Display song titles and musical keys for worship leaders and musicians. Update remotely from a Mac via web browser during services.

**Future:** Parallel display for ProPresenter operators.

### Requirements

- Hardware: ESP32 CYD (Cheap Yellow Display)
- Input: Web browser on macOS
- Latency: Seconds-level acceptable
- Content: 7-8 lines of text
- Updates: On-the-fly, remote

## Architecture

### Recommended: ESP32 Web Server

The ESP32 hosts its own web server. Users connect directly via browser.

```
Mac/Phone Browser  ---(WiFi/HTTP)-->  ESP32 CYD Web Server + Display
```

**Why this approach:**
- Self-contained, no external server needed
- Works on local network without internet
- Simple setup for church environment

## Implementation Plan

### Phase 1: Basic Web Server Display

1. Setup Development Environment
   - Install PlatformIO or Arduino IDE
   - Add ESP32 board support
   - Install libraries: TFT_eSPI, WiFi, WebServer

2. Hardware Setup
   - Get ESP32 CYD from Tony
   - Verify display pinout
   - Test basic display output

3. Web Server
   - Connect ESP32 to WiFi
   - Create web page with text input form
   - POST endpoint for text updates

4. Display
   - Text layout for 7-8 lines
   - Text wrapping
   - Readable fonts

### Phase 2: Enhanced Features

5. UI Improvements
   - Mobile-friendly interface
   - Preset key buttons (C, D, E, F, G, A, B)
   - Song + key template

6. Network
   - mDNS: `http://worship-display.local`
   - Show IP on boot
   - Auto-reconnect

### Phase 3: Multi-Display

7. MQTT for multiple displays
   - Local MQTT broker
   - Separate channels per display
   - ProPresenter integration

## Hardware: ESP32 CYD

- Display: 2.8" ILI9341 TFT (320x240)
- Touch: XPT2046 resistive
- Processor: ESP32-WROOM-32
- WiFi: 802.11 b/g/n

## Project Structure

```
a2n-arduino/
├── README.md
├── firmware/
│   ├── platformio.ini
│   └── src/
│       ├── main.cpp
│       ├── display.cpp
│       ├── webserver.cpp
│       └── config.h
├── data/
│   ├── index.html
│   ├── style.css
│   └── script.js
└── docs/
```

## Display Mockup

```
┌────────────────────────────┐
│                            │
│      CURRENT SONG          │
│                            │
│    "Amazing Grace"         │
│                            │
│       KEY: G               │
│                            │
│    Capo: 2nd fret          │
│                            │
└────────────────────────────┘
```

## Reference Projects

- [esp32-esl-system](https://github.com/giobauermeister/esp32-esl-system) - Web portal pattern
- [CYD-PhotoFrame](https://github.com/Grey-Lancaster/CYD-PhotoFrame) - Web upload interface
- [ESP32-Text-Reader](https://github.com/UnsignedArduino/ESP32-Text-Reader) - Text rendering

## Contributors

- Tony Chern - Project Owner
- Kentaro Vadney - Developer

# a2n-arduino

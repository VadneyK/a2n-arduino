# ESP32 Sticky Note Display

Firmware for the ESP32 "Cheap Yellow Display" (CYD) that turns it into a wireless sticky note. Anyone on the same Wi-Fi network can push a few lines of text to the screen from a browser or a one-line shell command. It was built to show song titles and keys to worship leaders and musicians during a service, but it displays whatever text you send.

- The display hosts its own web page, so no extra server is needed.
- Notes survive a reboot (stored in the ESP32's flash).
- If the configured Wi-Fi is unreachable, the display opens its own access point so it still works without a router.
- A GitHub Actions workflow compiles every pull request and publishes the firmware binaries.

## Hardware

The target is the ESP32-2432S028R, better known as the Cheap Yellow Display: an ESP32 with a built-in 2.8 inch 320 x 240 LCD, resistive touch screen and a USB port for power and programming ([CYD project README](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/README.md)).

### Wiring

There is nothing to wire. The display, touch controller and ESP32 are on one board; connect it to your computer with a micro USB cable ([CYD setup guide](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/SETUP.md)).

For reference, these are the on-board connections the firmware relies on. They are passed to the TFT_eSPI library as build flags in `platformio.ini`, so the library's stock `User_Setup.h` is left untouched. Values are taken from the CYD project's [User_Setup.h](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/DisplayConfig/User_Setup.h) and [PINS.md](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/PINS.md).

| Signal | ESP32 GPIO | Notes |
| --- | --- | --- |
| TFT MISO | 12 | HSPI bus |
| TFT MOSI | 13 | HSPI bus |
| TFT SCLK | 14 | HSPI bus |
| TFT CS | 15 | |
| TFT DC | 2 | |
| TFT RST | not connected | tied to the ESP32 reset |
| Backlight | 21 | active high |
| Touch CS | 33 | touch is on its own SPI bus and is not used by this firmware |

The display driver is the ILI9341 (TFT_eSPI's `ILI9341_2_DRIVER`), 240 x 320 in portrait. The firmware rotates it to landscape.

## Flashing

### 1. Install PlatformIO

The project builds with [PlatformIO Core](https://docs.platformio.org/en/latest/core/installation/index.html). The CLI is enough:

```bash
pip install platformio
```

The PlatformIO IDE extension for VS Code also works and reads the same `platformio.ini`.

### 2. Add your Wi-Fi credentials

Credentials live in `include/secrets.h`, which git ignores. Copy the example and edit it:

```bash
cp include/secrets.h.example include/secrets.h
```

```c
#define WIFI_SSID "your-network-name"
#define WIFI_PASSWORD "your-network-password"
```

If you skip this step the firmware still builds. At boot it will start an open access point called `sticky-note-setup` instead of joining a network.

### 3. Install the USB driver (first time only)

The CYD uses a CH340 USB to serial chip. If your computer does not see a serial port when the board is plugged in, install the driver following [Sparkfun's CH340 guide](https://learn.sparkfun.com/tutorials/how-to-install-ch340-drivers/all) ([CYD setup guide](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/SETUP.md)).

### 4. Build, upload, watch the log

```bash
pio run
```

```bash
pio run -t upload
```

```bash
pio device monitor
```

PlatformIO picks the serial port automatically. If it chooses the wrong one, add `--upload-port /dev/cu.usbserial-XXXX` (macOS) or the matching `/dev/ttyUSB0` on Linux. If uploads fail, lower `upload_speed` in `platformio.ini` to `115200`, which the CYD guide recommends when uploads error ([CYD setup guide](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display/blob/main/SETUP.md)).

The serial monitor prints the IP address once Wi-Fi connects. The screen footer shows the same address.

### Flashing a CI-built binary

Every CI run on `main` and every pull request uploads a `firmware-cyd` artifact containing `bootloader.bin`, `partitions.bin` and `firmware.bin`. These were built without `include/secrets.h`, so a board flashed with them starts the fallback access point. To flash them you need `esptool` (installed with PlatformIO under `~/.platformio/packages/tool-esptoolpy`) and the standard ESP32 Arduino offsets:

| File | Offset |
| --- | --- |
| bootloader.bin | 0x1000 |
| partitions.bin | 0x8000 |
| firmware.bin | 0x10000 |

Offsets are those the ESP32 Arduino framework hands to PlatformIO (see `FLASH_EXTRA_IMAGES` in [platformio-build.py](https://github.com/espressif/arduino-esp32/blob/2.0.17/tools/platformio-build.py) and `ESP32_APP_OFFSET` in the platform's [main.py](https://github.com/platformio/platform-espressif32/blob/v6.13.0/builder/main.py)). PlatformIO also writes `boot_app0.bin` at `0xe000`; `pio run -t upload` does all of this for you, so the artifact is mainly useful for a quick test flash.

## Using the display

### Web page

Open `http://sticky-note.local/` in a browser on the same network. The page has a text box, key buttons (C, D, E, F, G, A, B) that set a `Key:` line, and Send and Clear buttons. It loads the note currently on the screen so you can edit it.

The hostname is advertised with mDNS. If `.local` names do not resolve on your computer, use the IP address shown in the screen footer instead.

### Shell script

`tools/send-note.sh` wraps the HTTP calls with curl. Each argument becomes one line on the screen; with no arguments it reads the note from standard input.

```bash
tools/send-note.sh "Amazing Grace" "Key: G" "Capo 2"
```

```bash
echo "Break until 10:45" | tools/send-note.sh
```

```bash
tools/send-note.sh --clear
```

```bash
tools/send-note.sh --get
```

Use `-H 192.168.1.20` or set `STICKY_NOTE_HOST` to talk to a display by IP address or under another name.

### Fallback access point

When the display cannot join the configured network within 20 seconds, or `include/secrets.h` was not provided at build time, it starts an open Wi-Fi access point named `sticky-note-setup`. Join it and open the address shown in the footer (`192.168.4.1` by default for the ESP32 soft AP, as reported by the firmware's `/status` endpoint). The access point has no password, so use it only when you control the surroundings.

## Update protocol

The display speaks plain HTTP on port 80. All text is UTF-8; TFT_eSPI's built-in fonts only draw ASCII, so other characters render as blanks.

| Method | Path | Body | Response |
| --- | --- | --- | --- |
| GET | `/` | none | the control web page |
| GET | `/note` | none | `200`, current note as `text/plain` |
| POST | `/note` | form field `text` (`application/x-www-form-urlencoded`), or a raw body with any other content type | `200 ok`, `400` if no text, `413` if longer than 512 bytes |
| POST | `/clear` | none | `200 ok`, screen cleared |
| GET | `/status` | none | `200`, JSON with `hostname`, `ip`, `rssi`, `uptime_s`, `note_length` |

Lines are separated by `\n`. The firmware word-wraps each line to the screen width, draws up to 8 lines, and drops the rest. Short notes (up to about 4 lines) are drawn at double size.

Examples with curl:

```bash
curl -X POST --data-urlencode "text=Amazing Grace
Key: G" http://sticky-note.local/note
```

```bash
curl -X POST -H "Content-Type: text/plain" --data-binary "Sound check 9:30" http://sticky-note.local/note
```

```bash
curl http://sticky-note.local/status
```

The raw-body form works because the ESP32 Arduino `WebServer` exposes a non-form request body as an argument named `plain` ([Parsing.cpp](https://github.com/espressif/arduino-esp32/blob/2.0.17/libraries/WebServer/src/Parsing.cpp)). The note is saved with the ESP32 `Preferences` library so it is restored after a power cycle ([Preferences docs](https://docs.espressif.com/projects/arduino-esp32/en/latest/api/preferences.html)).

## Project layout

```
a2n-arduino/
  .github/workflows/build.yml   CI: PlatformIO build + script syntax check
  include/secrets.h.example     template for Wi-Fi credentials
  platformio.ini                board, library and TFT pin configuration
  src/main.cpp                  boot, Wi-Fi, mDNS, note persistence
  src/config.h                  hostname, limits, fallback AP name
  src/display.cpp / .h          TFT drawing and word wrap
  src/note_server.cpp / .h      HTTP endpoints
  src/page.h                    the embedded control web page
  tools/send-note.sh            command-line sender
```

## Continuous integration

`.github/workflows/build.yml` runs on every push to `main` and every pull request. It installs PlatformIO Core with pip, caches the downloaded toolchain and platform packages, runs `pio run`, and uploads the resulting binaries as a workflow artifact. The job follows PlatformIO's documented GitHub Actions recipe ([PlatformIO docs](https://docs.platformio.org/en/latest/integration/ci/github-actions.html)). A second small job runs `bash -n` on the sender script.

The platform (`espressif32@6.13.0`) and the display library (`TFT_eSPI@2.5.43`) are pinned in `platformio.ini` so CI and local builds use identical versions.

## Troubleshooting

- **Colours look inverted (white background).** Uncomment the `TFT_INVERSION_ON` build flag in `platformio.ini`. This is a standard TFT_eSPI option ([TFT_eSPI User_Setup.h](https://github.com/Bodmer/TFT_eSPI/blob/master/User_Setup.h)).
- **No serial port appears.** Install the CH340 driver (see Flashing, step 3).
- **Upload errors.** Set `upload_speed = 115200` in `platformio.ini`.
- **`sticky-note.local` does not resolve.** Use the IP address from the screen footer or the serial monitor.
- **Stuck on "Connecting".** Check `include/secrets.h`; after 20 seconds the display falls back to its own access point and shows that address instead.

## Roadmap

Done: web page with text box and key buttons, mDNS name, address on screen, auto-reconnect, note persistence, fallback access point, CI build.

Ideas not yet started, carried over from the original plan: MQTT fan-out to several displays, and a parallel display for ProPresenter operators.

## Reference projects

- [esp32-esl-system](https://github.com/giobauermeister/esp32-esl-system), web portal pattern
- [CYD-PhotoFrame](https://github.com/Grey-Lancaster/CYD-PhotoFrame), web upload interface
- [ESP32-Text-Reader](https://github.com/UnsignedArduino/ESP32-Text-Reader), text rendering
- [ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display), board documentation and display configuration

## Contributors

- Tony Chern, project owner
- Kentaro Vadney, developer

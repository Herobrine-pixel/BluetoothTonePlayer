# BluetoothTonePlayer

**Bluetooth-controlled tone & melody player for Arduino (HC-05).**

Important: HC-05 uses Bluetooth SPP (serial profile). This library does **not** provide A2DP audio streaming. It interprets serial commands and plays tones/melodies using the Arduino `tone()` API.

## What you get
- Library source (src/)
- Arduino example (examples/BluetoothToneDemo)
- PlatformIO example (main.cpp)
- platformio.ini, library.json, library.properties
- README and LICENSE (MIT)

## Wiring
- HC-05 VCC -> 5V
- HC-05 GND -> GND
- HC-05 TX -> Arduino pin 2  (SoftwareSerial RX)
- HC-05 RX -> Arduino pin 3  (SoftwareSerial TX) — use a 1k/2k voltage divider if Arduino is 5V to avoid damaging HC-05 RX
- Speaker + -> Arduino pin 5 (PWM capable)
- Speaker - -> GND

> Note: For louder output or better volume control use a small amplifier (PAM8403) driven by Arduino pin (or directly by a Bluetooth A2DP module if you need music streaming).

## Commands (send from phone using a Bluetooth Terminal app)
- `TONE:<freq>` -> play tone indefinitely (until `STOP`)
- `TONE:<freq>:<duration_ms>` -> play tone for duration
- `MELODY:<note,dur;...>` -> play a melody (note names like C4,D4 or raw frequency). Each pair `note,duration` separated by `;`
  - Example: `MELODY:C4,300;D4,300;E4,600`
- `FX:BEEP` `FX:ALARM` `FX:UP` `FX:DOWN` -> built-in effects
- `VOLUME:<0-10>` -> set software volume (best-effort; hardware amp recommended)
- `SAVE:<id>:<melody>` -> save a preset (id 1..10)
- `PLAY:<id>` -> play saved preset
- `STOP` -> stop playing

## Example Arduino sketch
Open `examples/BluetoothToneDemo/BluetoothToneDemo.ino` in Arduino IDE and upload.

## PlatformIO
Use the provided `platformio.ini` and `examples/BluetoothToneDemo/main.cpp`.

## License
MIT

# ESP32 RFID HomeKit Lock Starter

This repository is a safe starter for building an `ESP32 + RC522 + HomeKit`
project.

It is designed to:

- expose the ESP32 as a HomeKit lock accessory
- read locally managed RFID tag UIDs with an RC522 reader
- unlock a relay output when an approved UID is scanned
- keep the HomeKit lock state in sync with local access events

It is explicitly not designed to:

- emulate Apple Wallet NFC credentials
- emulate Apple Home Key
- copy, replay, export, or forge access credentials

If your real goal is "build a HomeKit-capable lock controller with local RFID
trigger support", this repository is a good starting point.

## Why this project avoids Apple Wallet / Home Key emulation

The `RC522` is suitable for reading common ISO14443A / MIFARE-style tags for
local automation and access-control prototypes.

Apple Wallet Home Key credentials are part of a protected secure-credential
system. This project does not attempt to simulate or interact with those
credentials.

## Stack

- [HomeSpan](https://github.com/HomeSpan/HomeSpan) for HomeKit accessory support
- [miguelbalboa/rfid](https://github.com/miguelbalboa/rfid) for RC522 tag reads

## Current flow

1. The RC522 reads a tag UID.
2. The UID is compared against a local whitelist.
3. If authorized, the ESP32 drives a relay output for a short time.
4. The HomeKit lock state is updated.
5. The lock returns to the secured state after a timeout.

## Files

- `platformio.ini`: PlatformIO project config
- `include/app_config.example.h`: example pin and whitelist config
- `src/main.cpp`: main firmware

## Default wiring

Default SPI pins can be changed in `include/app_config.h`.

| RC522 | ESP32 |
| --- | --- |
| SDA / SS | GPIO 21 |
| SCK | GPIO 18 |
| MOSI | GPIO 23 |
| MISO | GPIO 19 |
| RST | GPIO 27 |
| 3.3V | 3.3V |
| GND | GND |

Other outputs:

- relay input: `GPIO 26`
- status LED: `GPIO 2`

Hardware notes:

- power the RC522 from `3.3V`
- use a transistor, MOSFET, or a logic-safe relay board if needed
- add proper isolation and power design before driving a real door strike

## Getting started

### 1. Create a local config

```powershell
Copy-Item .\include\app_config.example.h .\include\app_config.h
```

Then edit `include/app_config.h` and update at least:

- accessory name
- RC522 pins
- relay pin
- `kAuthorizedUids`

### 2. Install PlatformIO

This starter is organized as a PlatformIO project.

- install the PlatformIO IDE extension in VS Code
- or install [PlatformIO Core](https://platformio.org/install)

### 3. Build and upload

```powershell
pio run
pio run -t upload
pio device monitor
```

## Recording approved tag UIDs

Run the firmware, open the serial monitor, and scan a tag.

- approved tags print `Authorized tag`
- unknown tags print `Rejected tag`

Copy the UID into the whitelist in `include/app_config.h`:

```cpp
constexpr const char *kAuthorizedUids[] = {
    "DE AD BE EF",
    "12 34 56 78",
};
```

## Included features

- HomeKit lock accessory skeleton
- RC522 polling
- UID whitelist matching
- duplicate-scan suppression
- timed auto-relock
- serial logging

## Good next steps

- move the whitelist into NVS storage
- add an admin-card enrollment mode
- add buzzer and richer LED feedback
- add a door contact sensor
- add a small web UI for tag management
- add Home Assistant or MQTT integration
- add audit logging

## Summary

This repository is a safe starter for a local RFID-triggered HomeKit lock
controller. It is not an Apple Wallet or Home Key emulation project.

# Arduino Uno RFID Lock Starter

This repository is a safe starter for building an `Arduino Uno + RC522`
project.

It is designed to:

- read locally managed RFID tag UIDs with an RC522 reader
- unlock a relay output when an approved UID is scanned
- relock automatically after a timeout

It is explicitly not designed to:

- emulate Apple Wallet NFC credentials
- emulate Apple Home Key
- copy, replay, export, or forge access credentials

If your real goal is "build a local RFID-triggered lock controller", this
repository is a good starting point.

## Uno limitation

`Arduino Uno` is suitable for local RFID reads and relay control.

It is not a practical target for native HomeKit accessory support. If you want
real HomeKit integration later, keep the RFID logic on Uno and add an ESP32 or
another network-capable controller as the HomeKit side.

## Stack

- [miguelbalboa/rfid](https://github.com/miguelbalboa/rfid) for RC522 tag reads

## Current flow

1. The RC522 reads a tag UID.
2. The UID is compared against a local whitelist.
3. If authorized, the Uno drives a relay output for a short time.
4. The lock returns to the secured state after a timeout.

## Files

- `platformio.ini`: PlatformIO Uno project config
- `include/app_config.example.h`: example pin and whitelist config
- `src/main.cpp`: main firmware

## Default wiring

On the Uno, the SPI pins are fixed.

| RC522 | Arduino Uno |
| --- | --- |
| SDA / SS | D10 |
| SCK | D13 |
| MOSI | D11 |
| MISO | D12 |
| RST | D9 |
| 3.3V | 3.3V |
| GND | GND |

Other outputs:

- relay input: `D7`
- status LED: `D8`

Hardware notes:

- power the RC522 from `3.3V`
- use a transistor, MOSFET, or a logic-safe relay board if needed
- add proper isolation and power design before driving a real door strike
- do not feed the RC522 from `5V`

## Getting started

### 1. Create a local config

```powershell
Copy-Item .\include\app_config.example.h .\include\app_config.h
```

Then edit `include/app_config.h` and update at least:

- controller name
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

- approved tags unlock the relay and print the UID
- unknown tags print `Rejected tag`

Copy the UID bytes into the whitelist in `include/app_config.h`:

```cpp
constexpr AuthorizedUid kAuthorizedUids[] = {
    {4, {0xDE, 0xAD, 0xBE, 0xEF}},
    {4, {0x12, 0x34, 0x56, 0x78}},
};
```

## Included features

- RC522 polling
- UID whitelist matching
- duplicate-scan suppression
- timed auto-relock
- serial logging

## Good next steps

- move the whitelist into EEPROM storage
- add an admin-card enrollment mode
- add buzzer and richer LED feedback
- add a door contact sensor
- add audit logging

## Summary

This repository is a safe starter for a local RFID-triggered lock controller on
Arduino Uno. It is not an Apple Wallet or Home Key emulation project.

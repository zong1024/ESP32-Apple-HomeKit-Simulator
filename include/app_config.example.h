#pragma once

#include <Arduino.h>

namespace AppConfig {

constexpr char kControllerName[] = "Arduino Uno RFID Lock";

// On the Uno, SPI pins are fixed:
// SCK=13, MISO=12, MOSI=11
constexpr uint8_t kRfidSsPin = 10;
constexpr uint8_t kRfidRstPin = 9;

constexpr uint8_t kRelayPin = 7;
constexpr uint8_t kStatusLedPin = 8;

constexpr unsigned long kUnlockDurationMs = 5000;
constexpr unsigned long kDuplicateScanWindowMs = 1200;

struct AuthorizedUid {
  uint8_t size;
  uint8_t bytes[10];
};

constexpr AuthorizedUid kAuthorizedUids[] = {
    {4, {0xDE, 0xAD, 0xBE, 0xEF}},
    {4, {0x12, 0x34, 0x56, 0x78}},
};

constexpr size_t kAuthorizedUidCount =
    sizeof(kAuthorizedUids) / sizeof(kAuthorizedUids[0]);

}  // namespace AppConfig

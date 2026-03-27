#pragma once

#include <Arduino.h>

namespace AppConfig {

constexpr char kControllerName[] = "ESP32-C3 RFID Lock";

#if defined(ARDUINO_ARCH_ESP32)
// ESP32-C3 defaults chosen to avoid flash pins (GPIO12-17),
// USB-JTAG pins (GPIO18-19), UART bridge pins (GPIO20-21),
// and common strapping pins (GPIO2, GPIO8, GPIO9).
constexpr uint8_t kSpiSckPin = 4;
constexpr uint8_t kSpiMisoPin = 5;
constexpr uint8_t kSpiMosiPin = 6;
constexpr uint8_t kRfidSsPin = 7;
constexpr uint8_t kRfidRstPin = 10;
constexpr uint8_t kRelayPin = 3;
constexpr uint8_t kStatusLedPin = 1;
#else
// Arduino Uno fallback:
// SCK=13, MISO=12, MOSI=11
constexpr uint8_t kRfidSsPin = 10;
constexpr uint8_t kRfidRstPin = 9;
constexpr uint8_t kRelayPin = 7;
constexpr uint8_t kStatusLedPin = 8;
#endif

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

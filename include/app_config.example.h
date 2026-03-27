#pragma once

#include <Arduino.h>

namespace AppConfig {

constexpr char kAccessoryName[] = "ESP32 RFID Lock";
constexpr char kLockName[] = "Front Door";

constexpr uint8_t kSpiSckPin = 18;
constexpr uint8_t kSpiMisoPin = 19;
constexpr uint8_t kSpiMosiPin = 23;
constexpr uint8_t kRfidSsPin = 21;
constexpr uint8_t kRfidRstPin = 27;

constexpr uint8_t kRelayPin = 26;
constexpr uint8_t kStatusLedPin = 2;

constexpr unsigned long kUnlockDurationMs = 5000;
constexpr unsigned long kDuplicateScanWindowMs = 1200;

constexpr const char *kAuthorizedUids[] = {
    "DE AD BE EF",
    "12 34 56 78",
};

constexpr size_t kAuthorizedUidCount =
    sizeof(kAuthorizedUids) / sizeof(kAuthorizedUids[0]);

}  // namespace AppConfig

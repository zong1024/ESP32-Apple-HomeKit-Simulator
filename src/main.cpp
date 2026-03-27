#include <Arduino.h>
#include <MFRC522.h>
#include <SPI.h>

#if __has_include("app_config.h")
#include "app_config.h"
#else
#include "app_config.example.h"
#endif

namespace {

MFRC522 gRfidReader(AppConfig::kRfidSsPin, AppConfig::kRfidRstPin);
byte gLastUidBytes[10] = {0};
byte gLastUidSize = 0;
unsigned long gLastScanMs = 0;
bool gIsUnlocked = false;
unsigned long gUnlockedAtMs = 0;

void printUid(const MFRC522::Uid& uid) {
  for (byte i = 0; i < uid.size; ++i) {
    if (uid.uidByte[i] < 0x10) {
      Serial.print('0');
    }
    Serial.print(uid.uidByte[i], HEX);
    if (i + 1 < uid.size) {
      Serial.print(' ');
    }
  }
}

bool isAuthorizedUid(const MFRC522::Uid& uid) {
  for (size_t i = 0; i < AppConfig::kAuthorizedUidCount; ++i) {
    const AppConfig::AuthorizedUid& authorized = AppConfig::kAuthorizedUids[i];
    if (uid.size != authorized.size) {
      continue;
    }

    bool matched = true;
    for (byte j = 0; j < uid.size; ++j) {
      if (uid.uidByte[j] != authorized.bytes[j]) {
        matched = false;
        break;
      }
    }

    if (matched) {
      return true;
    }
  }

  return false;
}

bool isDuplicateScan(const MFRC522::Uid& uid, unsigned long nowMs) {
  if (uid.size != gLastUidSize) {
    return false;
  }

  if (nowMs - gLastScanMs >= AppConfig::kDuplicateScanWindowMs) {
    return false;
  }

  for (byte i = 0; i < uid.size; ++i) {
    if (uid.uidByte[i] != gLastUidBytes[i]) {
      return false;
    }
  }

  return true;
}

void rememberUid(const MFRC522::Uid& uid, unsigned long nowMs) {
  gLastUidSize = uid.size;
  for (byte i = 0; i < uid.size; ++i) {
    gLastUidBytes[i] = uid.uidByte[i];
  }
  gLastScanMs = nowMs;
}

void blinkStatusLed(uint8_t cycles, unsigned long pulseMs) {
  for (uint8_t i = 0; i < cycles; ++i) {
    digitalWrite(AppConfig::kStatusLedPin, HIGH);
    delay(pulseMs);
    digitalWrite(AppConfig::kStatusLedPin, LOW);
    delay(pulseMs);
  }
}

void unlockRelay(const char* source) {
  digitalWrite(AppConfig::kRelayPin, HIGH);
  digitalWrite(AppConfig::kStatusLedPin, HIGH);
  gIsUnlocked = true;
  gUnlockedAtMs = millis();
  Serial.print(F("[LOCK] Unlocked via "));
  Serial.println(source);
}

void lockRelay(const char* source) {
  digitalWrite(AppConfig::kRelayPin, LOW);
  digitalWrite(AppConfig::kStatusLedPin, LOW);
  gIsUnlocked = false;
  gUnlockedAtMs = 0;
  Serial.print(F("[LOCK] Locked via "));
  Serial.println(source);
}

void handleRfidScan() {
  if (!gRfidReader.PICC_IsNewCardPresent() || !gRfidReader.PICC_ReadCardSerial()) {
    return;
  }

  const unsigned long nowMs = millis();

  if (!isDuplicateScan(gRfidReader.uid, nowMs)) {
    Serial.print(F("[RFID] Scanned UID "));
    printUid(gRfidReader.uid);
    Serial.println();

    if (isAuthorizedUid(gRfidReader.uid)) {
      unlockRelay("authorized tag");
    } else {
      Serial.println(F("[RFID] Rejected tag"));
      blinkStatusLed(2, 120);
    }
  }

  rememberUid(gRfidReader.uid, nowMs);

  gRfidReader.PICC_HaltA();
  gRfidReader.PCD_StopCrypto1();
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(1000);

  pinMode(AppConfig::kRelayPin, OUTPUT);
  pinMode(AppConfig::kStatusLedPin, OUTPUT);
  digitalWrite(AppConfig::kRelayPin, LOW);
  digitalWrite(AppConfig::kStatusLedPin, LOW);

  SPI.begin();
  gRfidReader.PCD_Init();

  Serial.println(F("[BOOT] Arduino Uno RFID lock ready."));
  Serial.print(F("[BOOT] Controller: "));
  Serial.println(AppConfig::kControllerName);
  Serial.print(F("[BOOT] RC522 firmware 0x"));
  Serial.println(gRfidReader.PCD_ReadRegister(MFRC522::VersionReg), HEX);
}

void loop() {
  if (gIsUnlocked &&
      millis() - gUnlockedAtMs >= AppConfig::kUnlockDurationMs) {
    lockRelay("timeout");
  }

  handleRfidScan();
}

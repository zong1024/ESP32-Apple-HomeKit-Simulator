#include <Arduino.h>
#include <HomeSpan.h>
#include <MFRC522.h>
#include <SPI.h>

#if __has_include("app_config.h")
#include "app_config.h"
#else
#include "app_config.example.h"
#endif

namespace {

class RfidLockAccessory : public Service::LockMechanism {
 public:
  explicit RfidLockAccessory(uint8_t relayPin, uint8_t statusLedPin)
      : Service::LockMechanism(),
        relayPin_(relayPin),
        statusLedPin_(statusLedPin),
        currentState_(new Characteristic::LockCurrentState(1)),
        targetState_(new Characteristic::LockTargetState(1, true)) {
    pinMode(relayPin_, OUTPUT);
    pinMode(statusLedPin_, OUTPUT);
    digitalWrite(relayPin_, LOW);
    digitalWrite(statusLedPin_, LOW);
  }

  boolean update() override {
    const int requestedState = targetState_->getNewVal();

    if (requestedState == 0) {
      unlock("HomeKit", false);
    } else {
      lock("HomeKit", false);
    }

    return true;
  }

  void handleAuthorizedCard(const String& uid) {
    Serial.printf("[RFID] Authorized tag %s\n", uid.c_str());
    unlock(uid.c_str(), true);
  }

  void handleRejectedCard(const String& uid) {
    Serial.printf("[RFID] Rejected tag %s\n", uid.c_str());
    blinkStatusLed(2, 120);
  }

  void loop() override {
    if (!isUnlocked_) {
      return;
    }

    if (millis() - unlockedAtMs_ >= AppConfig::kUnlockDurationMs) {
      lock("timeout", true);
    }
  }

 private:
  void unlock(const char* source, bool syncTargetState) {
    digitalWrite(relayPin_, HIGH);
    digitalWrite(statusLedPin_, HIGH);
    isUnlocked_ = true;
    unlockedAtMs_ = millis();
    currentState_->setVal(0);
    if (syncTargetState) {
      targetState_->setVal(0);
    }
    Serial.printf("[LOCK] Unlocked via %s\n", source);
  }

  void lock(const char* source, bool syncTargetState) {
    digitalWrite(relayPin_, LOW);
    digitalWrite(statusLedPin_, LOW);
    isUnlocked_ = false;
    unlockedAtMs_ = 0;
    currentState_->setVal(1);
    if (syncTargetState) {
      targetState_->setVal(1);
    }
    Serial.printf("[LOCK] Locked via %s\n", source);
  }

  void blinkStatusLed(uint8_t cycles, unsigned long pulseMs) {
    for (uint8_t i = 0; i < cycles; ++i) {
      digitalWrite(statusLedPin_, HIGH);
      delay(pulseMs);
      digitalWrite(statusLedPin_, LOW);
      delay(pulseMs);
    }
  }

  const uint8_t relayPin_;
  const uint8_t statusLedPin_;
  bool isUnlocked_ = false;
  unsigned long unlockedAtMs_ = 0;
  SpanCharacteristic* currentState_;
  SpanCharacteristic* targetState_;
};

MFRC522 gRfidReader(AppConfig::kRfidSsPin, AppConfig::kRfidRstPin);
RfidLockAccessory* gLockAccessory = nullptr;
String gLastUid;
unsigned long gLastScanMs = 0;

String formatUid(const MFRC522::Uid& uid) {
  String formatted;

  for (byte i = 0; i < uid.size; ++i) {
    if (i > 0) {
      formatted += ' ';
    }

    if (uid.uidByte[i] < 0x10) {
      formatted += '0';
    }

    formatted += String(uid.uidByte[i], HEX);
  }

  formatted.toUpperCase();
  return formatted;
}

bool isAuthorizedUid(const String& uid) {
  for (size_t i = 0; i < AppConfig::kAuthorizedUidCount; ++i) {
    if (uid.equalsIgnoreCase(AppConfig::kAuthorizedUids[i])) {
      return true;
    }
  }

  return false;
}

bool isDuplicateScan(const String& uid, unsigned long nowMs) {
  return uid == gLastUid &&
         nowMs - gLastScanMs < AppConfig::kDuplicateScanWindowMs;
}

void handleRfidScan() {
  if (!gRfidReader.PICC_IsNewCardPresent() || !gRfidReader.PICC_ReadCardSerial()) {
    return;
  }

  const unsigned long nowMs = millis();
  const String uid = formatUid(gRfidReader.uid);

  if (!isDuplicateScan(uid, nowMs)) {
    if (isAuthorizedUid(uid)) {
      gLockAccessory->handleAuthorizedCard(uid);
    } else {
      gLockAccessory->handleRejectedCard(uid);
    }
  }

  gLastUid = uid;
  gLastScanMs = nowMs;

  gRfidReader.PICC_HaltA();
  gRfidReader.PCD_StopCrypto1();
}

}  // namespace

void setup() {
  Serial.begin(115200);
  delay(1000);

  SPI.begin(AppConfig::kSpiSckPin, AppConfig::kSpiMisoPin,
            AppConfig::kSpiMosiPin, AppConfig::kRfidSsPin);
  gRfidReader.PCD_Init();

  homeSpan.setLogLevel(1);
  homeSpan.enableOTA();
  homeSpan.begin(Category::Locks, AppConfig::kAccessoryName);

  new SpanAccessory();
    new Service::AccessoryInformation();
      new Characteristic::Identify();
      new Characteristic::Name(AppConfig::kLockName);
      new Characteristic::Manufacturer("zong1024");
      new Characteristic::SerialNumber("ESP32-RC522-001");
      new Characteristic::Model("ESP32 RFID HomeKit Lock");
      new Characteristic::FirmwareRevision("0.1.0");

    gLockAccessory =
        new RfidLockAccessory(AppConfig::kRelayPin, AppConfig::kStatusLedPin);

  Serial.println("[BOOT] RFID HomeKit lock ready.");
  Serial.printf("[BOOT] RC522 firmware 0x%02X\n", gRfidReader.PCD_ReadRegister(MFRC522::VersionReg));
}

void loop() {
  homeSpan.poll();
  handleRfidScan();
}

#include "TDeckPlusKeyboard.h"

#include <Arduino.h>

#if !defined(HW_TDECK_PLUS)

// No-op stubs for non T-Deck Plus builds
void TDeckPlusKeyboard::begin(Display* /*display*/, UiInput* /*uiInput*/) {}
void TDeckPlusKeyboard::tick() {}

#else

#include <Wire.h>

// Pins per LilyGO T-Deck Plus doc
static constexpr int kBoardPowerOn   = 10;
static constexpr int kBoardI2cSda    = 18;
static constexpr int kBoardI2cScl    = 8;
static constexpr int kBoardKbdInt    = 46;

static constexpr uint32_t kRescanMs = 5000;
static constexpr uint32_t kI2cHz    = 400000;

bool TDeckPlusKeyboard::readActiveLow_(int pin) const {
  if (pin < 0) return false;
  return digitalRead(pin) == LOW;
}

void TDeckPlusKeyboard::dumpBytes_(const uint8_t* data, size_t n) const {
  Serial.print("kbd i2c rx ");
  Serial.print((int)n);
  Serial.print(" bytes: ");
  for (size_t i = 0; i < n; i++) {
    if (data[i] < 16) Serial.print('0');
    Serial.print(data[i], HEX);
    Serial.print(i + 1 == n ? "" : " ");
  }
  Serial.println();
}

uint8_t TDeckPlusKeyboard::scanForFirstDevice_() {
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    uint8_t err = Wire.endTransmission();
    if (err == 0) {
      Serial.print("i2c device found at 0x");
      if (addr < 16) Serial.print('0');
      Serial.println(addr, HEX);
      return addr;
    }
  }
  Serial.println("i2c scan: no devices found");
  return 0;
}

void TDeckPlusKeyboard::ensureI2cAndDetect_() {
  uint32_t now = millis();
  if (kbdAddr_ != 0) return;
  if (lastScanMs_ != 0 && (now - lastScanMs_) < kRescanMs) return;
  lastScanMs_ = now;
  kbdAddr_ = scanForFirstDevice_();
}

void TDeckPlusKeyboard::begin(Display* display, UiInput* uiInput) {
  ui_ = display;
  q_ = uiInput;

  pinPower_ = kBoardPowerOn;
  pinInt_ = kBoardKbdInt;

  pinMode(pinPower_, OUTPUT);
  digitalWrite(pinPower_, HIGH);

  pinMode(pinInt_, INPUT_PULLUP);

  Wire.begin(kBoardI2cSda, kBoardI2cScl);
  Wire.setClock(kI2cHz);

  Serial.println("TDeckPlusKeyboard: begin");
  ensureI2cAndDetect_();
}

void TDeckPlusKeyboard::tick() {
  ensureI2cAndDetect_();
  if (kbdAddr_ == 0) return;

  if (!readActiveLow_(pinInt_)) return;

  uint8_t buf[32] = {0};

  size_t got = 0;
  Wire.requestFrom((int)kbdAddr_, (int)sizeof(buf));
  while (Wire.available() && got < sizeof(buf)) {
    buf[got++] = (uint8_t)Wire.read();
  }

  if (got > 0) {
    dumpBytes_(buf, got);
  }
}

#endif
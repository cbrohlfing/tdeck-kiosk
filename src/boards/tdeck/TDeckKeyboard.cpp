#include "TDeckKeyboard.h"

#include <Arduino.h>

#if !defined(HW_TDECK) && !defined(HW_TDECK_PLUS)

// No-op stubs for non T-Deck builds
void TDeckKeyboard::begin(Display* /*display*/, UiInput* /*uiInput*/) {}
void TDeckKeyboard::tick() {}

#else

#include <Wire.h>
#include <string.h>

// Pins per LilyGO T-Deck Plus doc (matches your wiring)
static constexpr int kBoardPowerOn = 10;
static constexpr int kBoardI2cSda  = 18;
static constexpr int kBoardI2cScl  = 8;
static constexpr int kBoardKbdInt  = 46;

// NOTE: On T-Deck Plus, the keyboard controller is on I2C 0x55.
// We do *not* scan the bus anymore; we just probe this known address.
#if defined(HW_TDECK_PLUS)
static constexpr uint8_t kFixedKbdAddr = 0x55;
#else
// If you ever build for non-Plus, you can adjust this if needed.
static constexpr uint8_t kFixedKbdAddr = 0x55;
#endif

static constexpr uint32_t kI2cHz    = 100000;

// Polling cadence
static constexpr uint32_t kPollMs   = 10;

// Probe cadence (when keyboard isn't responding yet)
static constexpr uint32_t kRescanMs = 500;

// Prevent “infinite same-char” floods if the device gets into a weird state.
// Still allows normal key-repeat at a sane rate.
static constexpr uint32_t kRepeatGuardMs = 180;

// ---- Helpers ----

static bool probeI2cAddr(uint8_t addr) {
  Wire.beginTransmission((int)addr);
  return Wire.endTransmission() == 0;
}

bool TDeckKeyboard::readActiveLow_(int pin) const {
  if (pin < 0) return false;
  return digitalRead(pin) == LOW;
}

void TDeckKeyboard::dumpBytes_(const uint8_t* data, size_t n) const {
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

// Legacy scan helper (kept for troubleshooting). Not used by default.
uint8_t TDeckKeyboard::scanForFirstDevice_() {
  uint8_t first = 0;
  Serial.println("i2c scan:");
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    uint8_t err = Wire.endTransmission();
    if (err == 0) {
      Serial.print("  - 0x");
      if (addr < 16) Serial.print('0');
      Serial.println(addr, HEX);
      if (first == 0) first = addr;
    }
  }
  if (first == 0) Serial.println("  (none)");
  return first;
}

void TDeckKeyboard::ensureI2cAndDetect_() {
  // We no longer scan; we only probe the known address.
  // This avoids “wrong address” races and keeps things simple.
  if (kbdAddr_ == kFixedKbdAddr) return;

  uint32_t now = millis();
  if (lastScanMs_ != 0 && (now - lastScanMs_) < kRescanMs) return;
  lastScanMs_ = now;

  if (probeI2cAddr(kFixedKbdAddr)) {
    kbdAddr_ = kFixedKbdAddr;
    Serial.print("[kbd] keyboard ready at 0x");
    if (kbdAddr_ < 16) Serial.print('0');
    Serial.println(kbdAddr_, HEX);
  }
}

// Reads 16 bytes starting at register `reg` into out[16].
// (Kept for reference; not used by the FIFO-style read below.)
bool TDeckKeyboard::readReg16_(uint8_t reg, uint8_t out[16]) {
  if (kbdAddr_ == 0) return false;

  Wire.beginTransmission((int)kbdAddr_);
  Wire.write(reg);
  uint8_t err = Wire.endTransmission(false);
  if (err != 0) return false;

  memset(out, 0, 16);
  Wire.requestFrom((int)kbdAddr_, 16);

  size_t got = 0;
  while (Wire.available() && got < 16) {
    out[got++] = (uint8_t)Wire.read();
  }
  return got > 0;
}

void TDeckKeyboard::postEvent_(UiInputEvent e) {
  if (!q_) return;
  q_->post(e);
}

// ---- Public API ----

void TDeckKeyboard::begin(Display* display, UiInput* uiInput) {
  ui_ = display;
  q_  = uiInput;

  pinPower_ = kBoardPowerOn;
  pinInt_   = kBoardKbdInt;

  pinMode(pinPower_, OUTPUT);
  digitalWrite(pinPower_, HIGH);

  pinMode(pinInt_, INPUT_PULLUP);

  Wire.begin(kBoardI2cSda, kBoardI2cScl);
  Wire.setClock(kI2cHz);

  Serial.println("TDeckKeyboard: begin");
  Serial.println("[kbd] powering peripherals, waiting for keyboard...");

  // Start “unknown” and probe in tick()
  kbdAddr_ = 0;
  lastScanMs_ = 0;

  lastPollMs_ = 0;
  lastCode_   = 0;
  lastDown_   = false;
}

void TDeckKeyboard::tick() {
  // 1) Ensure keyboard is present
  if (kbdAddr_ == 0) {
    ensureI2cAndDetect_();
    return;
  }

  // 2) Poll cadence
  uint32_t now = millis();
  if (now - lastPollMs_ < kPollMs) return;
  lastPollMs_ = now;

  // 3) Read ONE byte from the keyboard as a FIFO stream.
  //    On your T-Deck Plus, this is the mode that produced correct ASCII output.
  Wire.requestFrom((int)kbdAddr_, 1);
  if (!Wire.available()) return;

  uint8_t b = (uint8_t)Wire.read();

  // 4) Ignore “no data” markers
  if (b == 0x00 || b == 0xFF) return;

  // 5) Guard against pathological fast repeats (e.g. endless '2')
  static uint8_t  lastB = 0;
  static uint32_t lastBms = 0;
  if (b == lastB && (now - lastBms) < kRepeatGuardMs) {
    return;
  }
  lastB = b;
  lastBms = now;

  // 6) Debug print
  Serial.print("[kbd] byte=0x");
  if (b < 16) Serial.print('0');
  Serial.print(b, HEX);
  if (b >= 32 && b <= 126) {
    Serial.print(" '");
    Serial.write((char)b);
    Serial.print("'");
  }
  Serial.println();

  // 7) Map some common navigation keys (optional)
  // NOTE: Many keys come through as ASCII already; arrows may be non-ASCII depending on firmware.
  UiInputEvent ev = UiInputEvent::None;
  switch (b) {
    // If your keyboard firmware emits nav codes, map them here once discovered.
    default: break;
  }
  if (ev != UiInputEvent::None) postEvent_(ev);

  // 8) Echo onto Display (single line) without letting it explode
  static String lineBuf;
  static constexpr size_t kMaxLine = 28;

  if (b == 0x08) { // backspace
    if (lineBuf.length() > 0) lineBuf.remove(lineBuf.length() - 1);
  } else if (b == 0x0D || b == 0x0A) { // enter/newline
    lineBuf = "";
  } else if (b >= 32 && b <= 126) {
    lineBuf += (char)b;
    if (lineBuf.length() > kMaxLine) {
      lineBuf = lineBuf.substring(lineBuf.length() - kMaxLine);
    }
  }

  if (ui_) {
    ui_->clear();
    ui_->line("Keyboard Debug");
    ui_->line(lineBuf);
  }
}

#endif
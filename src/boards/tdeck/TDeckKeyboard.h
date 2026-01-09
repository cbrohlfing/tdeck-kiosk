#pragma once

#include <Arduino.h>

#include "../../hw/Display.h"
#include "../../hw/UiInput.h"

// T-Deck keyboard scaffold.
//
// What it does today (safe without hardware):
// - Powers on peripherals (BOARD_POWERON)
// - Initializes I2C (BOARD_I2C_SDA / BOARD_I2C_SCL)
// - Watches keyboard INT pin (BOARD_KEYBOARD_INT)
// - If an I2C device is detected, tries to read bytes when INT triggers and
//   prints a hex dump to Serial.
//
// What we'll do later (once hardware arrives):
// - Confirm the keyboard's I2C address and packet format
// - Map keys to UiInputEvent (NavInput) and to text/lines (TextInput)
class TDeckKeyboard {
public:
  void begin(Display* display, UiInput* uiInput);
  void tick();

private:
  Display* ui_ = nullptr;
  UiInput* q_ = nullptr;

  int pinPower_ = -1;
  int pinInt_ = -1;

  uint8_t kbdAddr_ = 0;
  uint32_t lastScanMs_ = 0;

  bool readActiveLow_(int pin) const;
  void ensureI2cAndDetect_();
  uint8_t scanForFirstDevice_();
  void dumpBytes_(const uint8_t* data, size_t n) const;
};
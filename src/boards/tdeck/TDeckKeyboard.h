#pragma once

#include <Arduino.h>

#include "../../hw/Display.h"
#include "../../hw/UiInput.h"
#include "../../hw/UiInputEvent.h"

// T-Deck keyboard scaffold.
//
// What it does today:
// - Powers on peripherals
// - Initializes I2C
// - Polls the keyboard over I2C and emits UiInputEvent events into UiInput
class TDeckKeyboard {
public:
  void begin(Display* display, UiInput* uiInput);
  void tick();

private:
  Display* ui_ = nullptr;
  UiInput* q_  = nullptr;

  int pinPower_ = -1;
  int pinInt_   = -1;

  uint8_t  kbdAddr_    = 0;
  uint32_t lastScanMs_ = 0;

  // Key state tracking
  uint32_t lastPollMs_ = 0;
  uint8_t  lastCode_   = 0;
  bool     lastDown_   = false;

  bool readReg16_(uint8_t reg, uint8_t out[16]);
  void postEvent_(UiInputEvent e);

  bool readActiveLow_(int pin) const;
  void ensureI2cAndDetect_();
  uint8_t scanForFirstDevice_();
  void dumpBytes_(const uint8_t* data, size_t n) const;
};
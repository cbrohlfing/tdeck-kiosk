// /src/boards/heltec_v3/PowerButtonHeltecV3.h
#pragma once
#include <Arduino.h>

class Display;
class UiInput;

class PowerButtonHeltecV3 {
public:
  PowerButtonHeltecV3() = default;

  void begin(Display* display, UiInput* uiInput);
  void tick();

private:
  // Heltec V3 "PRG/BOOT" button is typically GPIO0 (active-low)
  static constexpr int BTN_PIN = 0;

  // Debounce
  static constexpr uint32_t DEBOUNCE_MS = 25;

  // UI press windows:
  // - release < SELECT_PRESS_MS  => Next (scroll)
  // - release >= SELECT_PRESS_MS => Select (choose)
  static constexpr uint32_t SELECT_PRESS_MS = 650;

  // Deep sleep window (very long press):
  // - hold >= SLEEP_ARM_MS => arm ("release to sleep")
  // - keep holding past (SLEEP_ARM_MS + TOO_LONG_MS) => abort (won't sleep)
  static constexpr uint32_t SLEEP_ARM_MS = 2200;
  static constexpr uint32_t TOO_LONG_MS = 1200;

  Display* _display = nullptr;
  UiInput* _uiInput = nullptr;

  // Debounce state
  bool _lastRawPressed = false;
  bool _stablePressed = false;
  uint32_t _lastChangeMs = 0;

  // Hold tracking
  uint32_t _pressStartMs = 0;

  // Sleep arming state
  bool _sleepArmed = false;
  bool _sleepAborted = false;
  uint32_t _lastReportMs = 0;

  bool rawPressed() const;   // reads pin, active-low
  void goToDeepSleep();
};
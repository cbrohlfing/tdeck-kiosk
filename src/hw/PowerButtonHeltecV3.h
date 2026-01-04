#pragma once
#include <Arduino.h>

class Display;

class PowerButtonHeltecV3 {
public:
  PowerButtonHeltecV3() = default;

  void begin(Display* display);
  void tick();

private:
  // Heltec V3 "PRG/BOOT" button is typically GPIO0 (active-low)
  static constexpr int BTN_PIN = 0;

  // Debounce
  static constexpr uint32_t DEBOUNCE_MS = 25;

  // Long-press window:
  // - hold >= LONG_PRESS_MS => arm ("release to sleep")
  // - keep holding past (LONG_PRESS_MS + TOO_LONG_MS) => abort (won't sleep)
  static constexpr uint32_t LONG_PRESS_MS = 1200;
  static constexpr uint32_t TOO_LONG_MS  = 1200;

  Display* _display = nullptr;

  // Debounce state
  bool _lastRawPressed = false;
  bool _stablePressed = false;
  uint32_t _lastChangeMs = 0;

  // Long-press state
  uint32_t _pressStartMs = 0;
  bool _armed = false;
  bool _aborted = false;
  uint32_t _lastReportMs = 0;

  bool rawPressed() const;   // reads pin, active-low
  void goToDeepSleep();
};
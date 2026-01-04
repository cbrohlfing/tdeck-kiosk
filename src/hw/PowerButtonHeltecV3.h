#pragma once
#include <Arduino.h>

class Display;

class PowerButtonHeltecV3 {
public:
  PowerButtonHeltecV3() = default;

  // Call once at boot
  void begin(Display* display);

  // Call frequently from loop()
  void tick();

private:
  Display* _display = nullptr;

  // Heltec V3 BOOT/PRG button is typically GPIO0 (active-low)
  static constexpr int BTN_PIN = 0;

  // Long press duration to trigger sleep
  static constexpr uint32_t LONG_PRESS_MS = 1500;

  // Simple debounce
  static constexpr uint32_t DEBOUNCE_MS = 25;

  bool _stablePressed = false;
  bool _lastRawPressed = false;

  uint32_t _lastChangeMs = 0;
  uint32_t _pressStartMs = 0;

  bool rawPressed() const {
    // Active-low
    return digitalRead(BTN_PIN) == LOW;
  }

  void goToDeepSleep();
};
#include "PowerButtonHeltecV3.h"

#include "Display.h"

#include <esp_sleep.h>

void PowerButtonHeltecV3::begin(Display* display) {
  _display = display;

  pinMode(BTN_PIN, INPUT_PULLUP);

  _stablePressed = rawPressed();
  _lastRawPressed = _stablePressed;
  _lastChangeMs = millis();

  if (_stablePressed) {
    _pressStartMs = millis();
  } else {
    _pressStartMs = 0;
  }
}

void PowerButtonHeltecV3::tick() {
  const uint32_t now = millis();

  const bool raw = rawPressed();
  if (raw != _lastRawPressed) {
    _lastRawPressed = raw;
    _lastChangeMs = now;
  }

  // Debounce to update stable state
  if ((now - _lastChangeMs) >= DEBOUNCE_MS) {
    if (raw != _stablePressed) {
      _stablePressed = raw;

      if (_stablePressed) {
        // just became pressed
        _pressStartMs = now;
      } else {
        // just released
        _pressStartMs = 0;
      }
    }
  }

  // Long press triggers deep sleep
  if (_stablePressed && _pressStartMs != 0 && (now - _pressStartMs) >= LONG_PRESS_MS) {
    // Prevent retrigger if we somehow keep running
    _pressStartMs = 0;
    goToDeepSleep();
  }
}

void PowerButtonHeltecV3::goToDeepSleep() {
  Serial.println("[pwr] long press -> deep sleep");

  if (_display) {
    _display->line("[pwr] sleeping...");
  }

  // Small delay so OLED/serial can flush
  delay(50);
  Serial.flush();

  // Best-effort: allow wake on BOOT/PRG (GPIO0) low.
  // If the board/RTC routing supports it, pressing BOOT wakes from deep sleep.
  // Even if it doesn't, RESET always wakes.
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

  // GPIO0 wake (active-low). If unsupported on your exact S3 config, it won't hurt;
  // you'll still be able to wake using RESET.
  esp_sleep_enable_ext0_wakeup((gpio_num_t)GPIO_NUM_0, 0);

  // Optional: reduce leakage and stop CPU
  esp_deep_sleep_start();
}
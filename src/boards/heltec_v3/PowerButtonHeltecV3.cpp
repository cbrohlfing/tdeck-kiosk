#include "PowerButtonHeltecV3.h"

#include "../../hw/Display.h"
#include <esp_sleep.h>

static void logLine(Display* display, const String& s) {
  // If we have a Display (which includes SerialDisplay via MultiDisplay),
  // it already prints to serial monitor. Avoid Serial.println duplication.
  if (display) display->line(s);
  else Serial.println(s);
}

bool PowerButtonHeltecV3::rawPressed() const {
  // Active-low button
  return digitalRead(BTN_PIN) == LOW;
}

void PowerButtonHeltecV3::begin(Display* display) {
  _display = display;

  pinMode(BTN_PIN, INPUT_PULLUP);

  _stablePressed = rawPressed();
  _lastRawPressed = _stablePressed;
  _lastChangeMs = millis();

  _pressStartMs = _stablePressed ? millis() : 0;
  _armed = false;
  _aborted = false;
  _lastReportMs = 0;
}

void PowerButtonHeltecV3::tick() {
  const uint32_t now = millis();

  // --- Debounce raw input ---
  const bool raw = rawPressed();
  if (raw != _lastRawPressed) {
    _lastRawPressed = raw;
    _lastChangeMs = now;
  }

  // When stable long enough, accept the new state
  if ((now - _lastChangeMs) >= DEBOUNCE_MS) {
    if (raw != _stablePressed) {
      const bool wasArmed = _armed;
      const bool wasAborted = _aborted;

      _stablePressed = raw;

      if (_stablePressed) {
        // pressed
        _pressStartMs = now;
        _armed = false;
        _aborted = false;
        _lastReportMs = 0;
      } else {
        // released
        if (wasArmed && !wasAborted) {
          logLine(_display, "[pwr] release detected -> deep sleep");
          logLine(_display, "[pwr] entering deep sleep now");
          goToDeepSleep();
          return;
        }

        if (wasArmed && wasAborted) {
          logLine(_display, "[pwr] sleep canceled");
        }

        // Reset state
        _pressStartMs = 0;
        _armed = false;
        _aborted = false;
        _lastReportMs = 0;
      }
    }
  }

  // --- Long-press window logic ---
  if (!_stablePressed || _pressStartMs == 0) return;

  const uint32_t heldMs = now - _pressStartMs;

  // Arm once we hit the long press threshold
  if (!_armed && heldMs >= LONG_PRESS_MS) {
    _armed = true;
    _aborted = false;

    logLine(_display, "[pwr] long press -> deep sleep");
    logLine(_display, "[pwr] release to sleep");

    // Set to "now" so we don't immediately print a reminder on normal holds
    _lastReportMs = now;
  }

  // Abort if held too long AFTER arming
  if (_armed && !_aborted && heldMs >= (LONG_PRESS_MS + TOO_LONG_MS)) {
    _aborted = true;
    logLine(_display, "[pwr] held too long; abort sleep");
  }

  // Optional periodic reminder while armed (less spammy)
  // Increase interval so normal holds usually won't hit it.
  static constexpr uint32_t REMIND_MS = 2500;

  if (_armed && !_aborted && (now - _lastReportMs) > REMIND_MS) {
    _lastReportMs = now;
    logLine(_display, "[pwr] release to sleep");
  }
}

void PowerButtonHeltecV3::goToDeepSleep() {
  // Do NOT enable GPIO wake sources here, or the button/USB can instantly wake it.
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);

  delay(50);
  Serial.flush();

  esp_deep_sleep_start();
}
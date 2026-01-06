// /src/boards/heltec_v3/PowerButtonHeltecV3.cpp
#include "PowerButtonHeltecV3.h"

#include "../../hw/Display.h"
#include "../../hw/UiInput.h"
#include "../../hw/UiInputEvent.h"

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

void PowerButtonHeltecV3::begin(Display* display, UiInput* uiInput) {
  _display = display;
  _uiInput = uiInput;

  pinMode(BTN_PIN, INPUT_PULLUP);

  _stablePressed = rawPressed();
  _lastRawPressed = _stablePressed;
  _lastChangeMs = millis();

  _pressStartMs = _stablePressed ? millis() : 0;
  _sleepArmed = false;
  _sleepAborted = false;
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
      const bool wasSleepArmed = _sleepArmed;
      const bool wasSleepAborted = _sleepAborted;
      const uint32_t pressStart = _pressStartMs;

      _stablePressed = raw;

      if (_stablePressed) {
        // pressed
        _pressStartMs = now;
        _sleepArmed = false;
        _sleepAborted = false;
        _lastReportMs = 0;
      } else {
        // released
        const uint32_t heldMs = (pressStart == 0) ? 0 : (now - pressStart);

        // If sleep was armed and not aborted, go to deep sleep.
        if (wasSleepArmed && !wasSleepAborted) {
          logLine(_display, "[pwr] release detected -> deep sleep");
          logLine(_display, "[pwr] entering deep sleep now");
          goToDeepSleep();
          return;
        }

        // If sleep was armed but then aborted, just report cancellation.
        if (wasSleepArmed && wasSleepAborted) {
          logLine(_display, "[pwr] sleep canceled");
        } else {
          // Not a sleep action: treat as UI input (Next/Select).
          if (_uiInput) {
            if (heldMs >= SELECT_PRESS_MS) {
              _uiInput->post(UiInputEvent::Select);
              if (_display) _display->line("[ui] Select");
            } else if (heldMs > 0) {
              _uiInput->post(UiInputEvent::Next);
              if (_display) _display->line("[ui] Next");
            }
          }
        }

        // Reset state
        _pressStartMs = 0;
        _sleepArmed = false;
        _sleepAborted = false;
        _lastReportMs = 0;
      }
    }
  }

  // --- Sleep arming logic (very long press) ---
  if (!_stablePressed || _pressStartMs == 0) return;

  const uint32_t heldMs = now - _pressStartMs;

  // Arm once we hit the deep sleep threshold
  if (!_sleepArmed && heldMs >= SLEEP_ARM_MS) {
    _sleepArmed = true;
    _sleepAborted = false;

    logLine(_display, "[pwr] very long press -> deep sleep");
    logLine(_display, "[pwr] release to sleep");

    // Set to "now" so we don't immediately print a reminder on normal holds
    _lastReportMs = now;
  }

  // Abort if held too long AFTER arming
  if (_sleepArmed && !_sleepAborted && heldMs >= (SLEEP_ARM_MS + TOO_LONG_MS)) {
    _sleepAborted = true;
    logLine(_display, "[pwr] held too long; abort sleep");
  }

  // Optional periodic reminder while armed (less spammy)
  static constexpr uint32_t REMIND_MS = 2500;

  if (_sleepArmed && !_sleepAborted && (now - _lastReportMs) > REMIND_MS) {
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
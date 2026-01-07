// /src/boards/tdeck/TDeckTrackball.cpp
#include "TDeckTrackball.h"

#include <Arduino.h>

#include "../../hw/Display.h"
#include "../../hw/UiInput.h"
#include "../../hw/UiInputEvent.h"
#include "pins.h"

// IMPORTANT:
// PlatformIO compiles everything under /src for each environment.
// So we must compile to a no-op unless HW_TDECK is defined.
#if defined(HW_TDECK)

bool TDeckTrackball::readActiveLow_(int pin) const {
  if (pin < 0) return false;
  return digitalRead(pin) == LOW;
}

void TDeckTrackball::post_(uint8_t e) {
  if (!q_) return;
  q_->post((UiInputEvent)e);
}

void TDeckTrackball::begin(Display* display, UiInput* uiInput) {
  ui_ = display;
  q_ = uiInput;

  pinUp_    = (int)TDECK_TRACKBALL_UP_PIN;
  pinDown_  = (int)TDECK_TRACKBALL_DOWN_PIN;
  pinLeft_  = (int)TDECK_TRACKBALL_LEFT_PIN;
  pinRight_ = (int)TDECK_TRACKBALL_RIGHT_PIN;
  pinPress_ = (int)TDECK_TRACKBALL_PRESS_PIN;

  auto setupPin = [](int pin) {
    if (pin >= 0) pinMode(pin, INPUT_PULLUP);
  };

  setupPin(pinUp_);
  setupPin(pinDown_);
  setupPin(pinLeft_);
  setupPin(pinRight_);
  setupPin(pinPress_);

  // initialize last states
  lastUp_    = readActiveLow_(pinUp_);
  lastDown_  = readActiveLow_(pinDown_);
  lastLeft_  = readActiveLow_(pinLeft_);
  lastRight_ = readActiveLow_(pinRight_);

  pressDown_ = false;
  longFired_ = false;
  pressAtMs_ = 0;
  lastEventMs_ = 0;

  if (ui_) ui_->line("[boot] TDeckTrackball init ok");
}

void TDeckTrackball::tick() {
  const uint32_t now = millis();

  auto allowEvent = [&]() -> bool {
    if ((now - lastEventMs_) < kDebounceMs) return false;
    lastEventMs_ = now;
    return true;
  };

  // Directional presses (trigger on press-down edge)
  if (pinUp_ >= 0) {
    const bool cur = readActiveLow_(pinUp_);
    if (!lastUp_ && cur && allowEvent()) post_((uint8_t)UiInputEvent::NavPrev);
    lastUp_ = cur;
  }

  if (pinDown_ >= 0) {
    const bool cur = readActiveLow_(pinDown_);
    if (!lastDown_ && cur && allowEvent()) post_((uint8_t)UiInputEvent::NavNext);
    lastDown_ = cur;
  }

  if (pinLeft_ >= 0) {
    const bool cur = readActiveLow_(pinLeft_);
    if (!lastLeft_ && cur && allowEvent()) post_((uint8_t)UiInputEvent::Back);
    lastLeft_ = cur;
  }

  if (pinRight_ >= 0) {
    const bool cur = readActiveLow_(pinRight_);
    // Right mapped to Select (optional redundancy with click)
    if (!lastRight_ && cur && allowEvent()) post_((uint8_t)UiInputEvent::Select);
    lastRight_ = cur;
  }

  // Click press: short = Select, long = Home
  if (pinPress_ >= 0) {
    const bool pressed = readActiveLow_(pinPress_);

    if (!pressDown_ && pressed) {
      pressDown_ = true;
      longFired_ = false;
      pressAtMs_ = now;
    }

    if (pressDown_ && pressed && !longFired_ && (now - pressAtMs_) >= kLongPressMs) {
      longFired_ = true;
      post_((uint8_t)UiInputEvent::Home);
    }

    if (pressDown_ && !pressed) {
      if (!longFired_ && allowEvent()) {
        post_((uint8_t)UiInputEvent::Select);
      }
      pressDown_ = false;
    }
  }
}

#else

// Non-TDeck builds: compile to no-ops so other environments (Heltec) stay green.
void TDeckTrackball::begin(Display*, UiInput*) {}
void TDeckTrackball::tick() {}

#endif
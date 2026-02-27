#pragma once

#include <Arduino.h>
#include "../hw/Display.h"
#include "../hw/UiInput.h"
#include "../hw/UiInputEvent.h"

class UiApp {
public:
  void begin(Display* display, UiInput* input);
  void loop();

private:
  Display* display_ = nullptr;
  UiInput* input_   = nullptr;

  // debug state
  UiInputEvent lastEvent_ = UiInputEvent::None;
  uint8_t lastKeycode_    = 0x00;
  uint32_t eventCount_    = 0;

  // redraw control
  bool dirty_ = true;
  uint32_t lastDrawMs_ = 0;

  void draw_();
  static const char* eventName_(UiInputEvent e);
};
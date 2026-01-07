// /src/boards/tdeck/TDeckTrackball.h
#pragma once
#include <stdint.h>

class Display;
class UiInput;

// Polling-based trackball input mapping for T-Deck.
// Posts UiInputEvent values into the UiInput queue.
class TDeckTrackball {
public:
  void begin(Display* display, UiInput* uiInput);
  void tick();

private:
  bool readActiveLow_(int pin) const;
  void post_(uint8_t e);

  Display* ui_ = nullptr;
  UiInput* q_ = nullptr;

  int pinUp_ = -1;
  int pinDown_ = -1;
  int pinLeft_ = -1;
  int pinRight_ = -1;
  int pinPress_ = -1;

  bool lastUp_ = false;
  bool lastDown_ = false;
  bool lastLeft_ = false;
  bool lastRight_ = false;

  bool pressDown_ = false;
  bool longFired_ = false;

  uint32_t pressAtMs_ = 0;
  uint32_t lastEventMs_ = 0;

  static constexpr uint16_t kDebounceMs = 40;
  static constexpr uint16_t kLongPressMs = 700;
};
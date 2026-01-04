#pragma once
#include <Arduino.h>

#include "Display.h"
#include "BatteryMonitor.h"

#include <Wire.h>
#include <U8g2lib.h>

// Heltec WiFi LoRa 32 (V3) built-in OLED pins (per your pinout / board)
// OLED: SDA=GPIO17, SCL=GPIO18, RST=GPIO21
// VEXT control is commonly GPIO36 on Heltec boards (powers OLED / external rail).
// On many Heltec boards VEXT is ACTIVE-LOW (LOW = ON). We'll do LOW=ON.
class OledDisplayHeltecV3 : public Display {
public:
  OledDisplayHeltecV3() = default;

  void begin(BatteryMonitor* batt);

  // Display interface
  void clear() override;
  void line(const String& s) override;

  // Call periodically so the top bar refreshes even when no new lines are printed.
  void tick();

private:
  // Pins
  static constexpr int OLED_SDA_PIN  = 17;
  static constexpr int OLED_SCL_PIN  = 18;
  static constexpr int OLED_RST_PIN  = 21;
  static constexpr int VEXT_PIN      = 36;

  // Refresh behavior
  static constexpr uint32_t TOPBAR_REFRESH_MS = 500;

  // 128x64 with 6x10 font: topbar uses y=10..12, content starts at ~24.
  static constexpr int MAX_LINES = 4;

  BatteryMonitor* _batt = nullptr;

  // U8g2 HW I2C (uses Wire), with reset pin
  U8G2_SSD1306_128X64_NONAME_F_HW_I2C _u8g2 = U8G2_SSD1306_128X64_NONAME_F_HW_I2C(
    U8G2_R0,
    /* reset = */ OLED_RST_PIN,
    /* clock = */ U8X8_PIN_NONE,
    /* data  = */ U8X8_PIN_NONE
  );

  String _lines[MAX_LINES];
  int _count = 0;

  uint32_t _lastTopbarRedrawMs = 0;
  String _lastTopbar;

  void redraw(bool force);
  static String sanitizeForOled(const String& in);

  void ensureOledPowerAndBus();
  void probeI2C();
};
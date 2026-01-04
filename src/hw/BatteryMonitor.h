#pragma once
#include <Arduino.h>

class BatteryMonitor {
public:
  BatteryMonitor() = default;

  void begin();
  void tick();

  float voltage() const { return _vbat; }     // battery voltage (V)
  int percent() const { return _pct; }        // 0-100 (heuristic)
  bool isCharging() const { return _charging; }

  // Debug helpers
  int lastRaw() const { return _raw; }
  int lastAdcMilliVolts() const { return _adcMv; }
  bool batteryPresent() const { return _present; }
  bool usbPresent() const { return _usbPresent; }

  // Short status for OLED top bar
  // Examples:
  //  - "3.92V 74% CHG"
  //  - "3.85V 62% BAT"
  //  - "USB (no batt)"
  //  - "VBAT N/A"
  String statusShort() const;

private:
  // Heltec WiFi LoRa 32 (V3) per your pin map:
  //  - VBAT_Read is ADC1_CH0 -> GPIO1
  //  - ADC_Ctrl is GPIO37 (enables the VBAT divider/sense path on many Heltec designs)
  //  - USB5V sense shown as GPIO38 on your pin map (may be board/variant dependent)
  static constexpr int VBAT_ADC_PIN = 1;
  static constexpr int ADC_CTRL_PIN = 37;
  static constexpr int USB5V_SENSE_PIN = 38;

  // Heltec V3 divider approx (can calibrate later)
  static constexpr float VBAT_DIVIDER = 4.90f;

  // Update timing
  static constexpr uint32_t SAMPLE_MS = 1000;

  // Charging inference window
  static constexpr uint32_t TREND_WINDOW_MS = 30000; // 30s
  static constexpr float CHG_RISE_V = 0.015f;        // 15mV

  // Battery present heuristic (ADC pin millivolts)
  static constexpr int PRESENT_ADC_MV_MIN = 50;

  float _vbat = 0.0f;
  int _pct = 0;
  bool _charging = false;
  bool _present = false;
  bool _usbPresent = false;

  int _raw = 0;
  int _adcMv = 0;

  uint32_t _lastSampleMs = 0;

  // Trend tracking
  uint32_t _trendStartMs = 0;
  float _trendStartV = 0.0f;

  static float clampf(float x, float a, float b) {
    if (x < a) return a;
    if (x > b) return b;
    return x;
  }

  static int clampi(int x, int a, int b) {
    if (x < a) return a;
    if (x > b) return b;
    return x;
  }

  static int estimatePercent(float vbat);
};
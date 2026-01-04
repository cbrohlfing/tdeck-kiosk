#include "BatteryMonitor.h"

#if defined(ESP32)
  #include <driver/adc.h>
#endif

static bool readUsbSensePin(int pin) {
  // Some boards may not wire this; reading it is harmless.
  // If it floats you might see random values; we’ll treat it as "best effort".
  int v = digitalRead(pin);
  return (v == HIGH);
}

void BatteryMonitor::begin() {
#if defined(ESP32)
  analogReadResolution(12);

  // ESP32-S3: per-pin attenuation tends to behave better than global attenuation.
  analogSetPinAttenuation((gpio_num_t)VBAT_ADC_PIN, ADC_11db);
#endif

  // Enable VBAT sense path if the board uses ADC_Ctrl gating
  pinMode(ADC_CTRL_PIN, OUTPUT);
  digitalWrite(ADC_CTRL_PIN, HIGH);

  // USB sense input (best effort; depends on board wiring)
  pinMode(USB5V_SENSE_PIN, INPUT);

  _lastSampleMs = 0;
  _trendStartMs = millis();
  _trendStartV = 0.0f;

  tick(); // initial sample
}

void BatteryMonitor::tick() {
  uint32_t now = millis();
  if (_lastSampleMs != 0 && (now - _lastSampleMs) < SAMPLE_MS) return;
  _lastSampleMs = now;

  // Always keep ADC_Ctrl enabled (some boards might glitch it low after sleep etc.)
  digitalWrite(ADC_CTRL_PIN, HIGH);

  // Best-effort USB present read
  _usbPresent = readUsbSensePin(USB5V_SENSE_PIN);

  const int N = 8;
  int rawSum = 0;
  int mvSum = 0;

  for (int i = 0; i < N; i++) {
    int raw = analogRead(VBAT_ADC_PIN);

#if defined(ESP32)
    int mv  = analogReadMilliVolts(VBAT_ADC_PIN); // calibrated reading when supported
#else
    // Fallback: approximate using raw scale (rarely used for this project)
    int mv = (int)((raw / 4095.0f) * 3300.0f);
#endif

    rawSum += raw;
    mvSum  += mv;

    delay(2);
  }

  _raw = rawSum / N;
  _adcMv = mvSum / N;

  // Decide if VBAT sense is "present"
  _present = (_adcMv >= PRESENT_ADC_MV_MIN);

  if (!_present) {
    // No valid VBAT reading.
    _vbat = 0.0f;
    _pct = 0;
    _charging = false;

    _trendStartMs = now;
    _trendStartV = 0.0f;
    return;
  }

  // Convert ADC millivolts at the pin to battery voltage (through divider)
  float v_adc = (float)_adcMv / 1000.0f;
  float v_bat = v_adc * VBAT_DIVIDER;

  // Clamp to sane LiPo range for display
  v_bat = clampf(v_bat, 3.0f, 4.35f);

  _vbat = v_bat;
  _pct = estimatePercent(_vbat);

  // Charging inference by trend (only if we have a valid reading)
  if (_trendStartMs == 0) {
    _trendStartMs = now;
    _trendStartV = _vbat;
    _charging = false;
    return;
  }

  if ((now - _trendStartMs) >= TREND_WINDOW_MS) {
    float dv = _vbat - _trendStartV;

    // If USB is present and VBAT is rising, call it charging.
    // If USB isn't present, we can still show CHG if it rises, but it’s less meaningful.
    _charging = (dv >= CHG_RISE_V);

    _trendStartMs = now;
    _trendStartV = _vbat;
  }
}

int BatteryMonitor::estimatePercent(float vbat) {
  if (vbat >= 4.20f) return 100;
  if (vbat <= 3.30f) return 0;

  if (vbat < 3.70f) {
    float t = (vbat - 3.30f) / (3.70f - 3.30f);
    int p = (int)(t * 50.0f);
    return clampi(p, 0, 50);
  } else {
    float t = (vbat - 3.70f) / (4.20f - 3.70f);
    int p = 50 + (int)(t * 50.0f);
    return clampi(p, 50, 100);
  }
}

String BatteryMonitor::statusShort() const {
  if (!_present) {
    if (_usbPresent) return "USB (no batt)";
    return "VBAT N/A";
  }

  String s;
  s.reserve(20);
  s += String(_vbat, 2);
  s += "V ";
  s += String(_pct);
  s += "% ";

  // If USB sense says present, lean toward CHG vs BAT wording
  if (_usbPresent) {
    s += (_charging ? "CHG" : "USB");
  } else {
    s += (_charging ? "CHG" : "BAT");
  }

  return s;
}
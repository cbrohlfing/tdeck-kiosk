#include "OledDisplayHeltecV3.h"

static void shiftUp(String* lines, int n) {
  for (int i = 0; i < n - 1; i++) lines[i] = lines[i + 1];
  lines[n - 1] = "";
}

String OledDisplayHeltecV3::sanitizeForOled(const String& in) {
  // OLED fonts may not show emoji/unicode; keep printable ASCII
  String out;
  out.reserve(in.length());
  for (size_t i = 0; i < in.length(); i++) {
    char c = in[i];
    if (c >= 32 && c <= 126) out += c;
    else out += ' ';
  }
  return out;
}

void OledDisplayHeltecV3::ensureOledPowerAndBus() {
  // Power the OLED rail (VEXT)
  pinMode(VEXT_PIN, OUTPUT);
  digitalWrite(VEXT_PIN, LOW); // Heltec commonly uses LOW = enable VEXT
  delay(10);

  // Make sure OLED reset is released
  pinMode(OLED_RST_PIN, OUTPUT);
  digitalWrite(OLED_RST_PIN, HIGH);
  delay(5);

  // Force I2C pins (some libs/cores may start Wire with defaults; ensure ours win)
  Wire.end();
  delay(2);
  Wire.begin(OLED_SDA_PIN, OLED_SCL_PIN, 100000);
  delay(2);
}

void OledDisplayHeltecV3::probeI2C() {
  auto probe = [](uint8_t addr) -> bool {
    Wire.beginTransmission(addr);
    return (Wire.endTransmission() == 0);
  };

  bool a3c = probe(0x3C);
  bool a3d = probe(0x3D);

  Serial.print("[oled] I2C probe: 0x3C=");
  Serial.print(a3c ? "YES" : "no");
  Serial.print(" 0x3D=");
  Serial.println(a3d ? "YES" : "no");
}

void OledDisplayHeltecV3::begin(BatteryMonitor* batt) {
  _batt = batt;

  ensureOledPowerAndBus();
  probeI2C();

  _u8g2.begin();
  _u8g2.setFont(u8g2_font_6x10_tf);

  // Clear buffer + show boot line
  _count = 0;
  for (int i = 0; i < MAX_LINES; i++) _lines[i] = "";

  _lastTopbar = "";
  _lastTopbarRedrawMs = 0;

  // Force first render
  line("[boot] OLED ready");
}

void OledDisplayHeltecV3::clear() {
  _count = 0;
  for (int i = 0; i < MAX_LINES; i++) _lines[i] = "";
  redraw(true);
}

void OledDisplayHeltecV3::line(const String& s) {
  String t = sanitizeForOled(s);

  if (_count >= MAX_LINES) {
    shiftUp(_lines, MAX_LINES);
    _lines[MAX_LINES - 1] = t;
  } else {
    _lines[_count++] = t;
  }

  redraw(true);
}

void OledDisplayHeltecV3::tick() {
  // Refresh top bar periodically even if no new log lines arrive
  uint32_t now = millis();
  if ((now - _lastTopbarRedrawMs) < TOPBAR_REFRESH_MS) return;

  _lastTopbarRedrawMs = now;
  redraw(false);
}

void OledDisplayHeltecV3::redraw(bool force) {
  _u8g2.clearBuffer();

  _u8g2.setFont(u8g2_font_6x10_tf);

  String top = (_batt ? _batt->statusShort() : String("VBAT N/A"));
  top = sanitizeForOled(top);

  // Only redraw if changed unless forced (saves a tiny bit of work)
  if (!force && top == _lastTopbar) {
    // Still need to draw the existing content; but if top is unchanged we can keep lastTopbar.
    // We still render because we cleared buffer; simplest is always render.
  }
  _lastTopbar = top;

  // Top bar
  _u8g2.drawStr(0, 10, top.c_str());
  _u8g2.drawHLine(0, 12, 128);

  // Content area
  int y = 24;
  for (int i = 0; i < _count && i < MAX_LINES; i++) {
    String ln = _lines[i];
    if (ln.length() > 21) ln = ln.substring(0, 21);
    _u8g2.drawStr(0, y, ln.c_str());
    y += 10;
  }

  _u8g2.sendBuffer();
}
#pragma once
#include "Display.h"

class SerialDisplay : public Display {
public:
  void clear() override {
    Serial.println();
    Serial.println("========================================");
  }

  void line(const String& s) override {
    Serial.println(s);
  }
};
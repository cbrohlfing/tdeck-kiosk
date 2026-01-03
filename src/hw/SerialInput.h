#pragma once
#include "Input.h"

class SerialInput : public Input {
public:
  bool pollLine(String& outLine) override {
    while (Serial.available() > 0) {
      char c = (char)Serial.read();
      if (c == '\r') continue;

      if (c == '\n') {
        outLine = buffer;
        buffer = "";
        outLine.trim();
        return true;
      }

      // Basic sanity: avoid huge lines
      if (buffer.length() < 64) buffer += c;
    }
    return false;
  }

private:
  String buffer;
};
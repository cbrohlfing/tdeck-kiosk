#include "SerialInput.h"

#include <Arduino.h>

SerialInput::SerialInput() {}

bool SerialInput::pollLine(String& outLine) {
  while (Serial.available() > 0) {
    int c = Serial.read();
    if (c < 0) break;

    // Handle newline (CR, LF, or CRLF)
    if (c == '\r' || c == '\n') {
      if (buffer.length() == 0) {
        continue;
      }
      outLine = buffer;
      buffer = "";
      return true;
    }

    // Backspace handling (BS or DEL)
    if (c == 0x08 || c == 0x7F) {
      if (buffer.length() > 0) {
        buffer.remove(buffer.length() - 1);

        // IMPORTANT:
        // monitor_echo=yes already moved cursor left
        // So we only erase the character and move left once
        Serial.print(" \b");
      }
      continue;
    }

    // Ignore other control characters
    if (c < 0x20) {
      continue;
    }

    // Normal character
    buffer += (char)c;

    // DO NOT echo here (PlatformIO does it already)
  }

  return false;
}
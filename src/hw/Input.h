#pragma once
#include <Arduino.h>

class Input {
public:
  virtual ~Input() = default;

  // Returns true when a full line (ending in \n) is available.
  virtual bool pollLine(String& outLine) = 0;
};
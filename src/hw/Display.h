#pragma once
#include <Arduino.h>

class Display {
public:
  virtual ~Display() = default;
  virtual void clear() = 0;
  virtual void line(const String& s) = 0;
};
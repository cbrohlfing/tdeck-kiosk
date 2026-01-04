#pragma once

#include <Arduino.h>
#include "Input.h"

class SerialInput : public Input {
 public:
  SerialInput();

  bool pollLine(String& outLine) override;

 private:
  String buffer;
};
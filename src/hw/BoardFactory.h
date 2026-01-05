#pragma once
#include <Arduino.h>

#include "BoardServices.h"

class BoardFactory {
public:
  static BoardServices begin();
};
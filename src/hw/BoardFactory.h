#pragma once
#include <Arduino.h>

#include "BoardServices.h"

class BoardFactory {
public:
  static BoardServices begin();

  // Board-level periodic work (battery monitor, power button, etc.)
  // Keeps UiApp board-agnostic.
  static void tick(BoardServices& hw);
};
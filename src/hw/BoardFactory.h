// /src/hw/BoardFactory.h
#pragma once

#include "BoardServices.h"

// Central place to construct/tick the concrete board services (display/input/uiInput/etc)
class BoardFactory {
public:
  static BoardServices begin();
  static void tick(BoardServices& hw);
};
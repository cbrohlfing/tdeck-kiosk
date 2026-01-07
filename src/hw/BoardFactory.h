// /src/hw/BoardFactory.h
#pragma once

#include "BoardServices.h"   // <-- Required: BoardServices is used by value + by reference

struct BoardFactory {
  static BoardServices begin();
  static void tick(BoardServices& hw);
};
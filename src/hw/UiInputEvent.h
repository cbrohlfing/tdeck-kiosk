// /src/hw/UiInputEvent.h
#pragma once
#include <stdint.h>

enum class UiInputEvent : uint8_t {
  None = 0,
  Next,     // short press
  Select,   // long press
};
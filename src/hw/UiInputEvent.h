#pragma once

enum class UiInputEvent {
  NONE = 0,
  NEXT,     // short press (scroll / move selection down)
  SELECT,   // long press (activate)
  BACK      // reserved for later (e.g., trackball click+hold, extra button, etc.)
};
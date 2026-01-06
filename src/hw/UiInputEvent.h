// /src/hw/UiInputEvent.h
#pragma once
#include <stdint.h>

// Board-agnostic UI navigation events.
// Boards emit these events (button/trackball/etc). App/UI consumes them.
//
// Intent:
// - Heltec (single button): short press -> NavNext, long press -> Home (or Back depending on screen)
// - T-Deck (trackball): up/down -> NavPrev/NavNext, click -> Select, long click -> Home, etc.
enum class UiInputEvent : uint8_t {
  None = 0,

  // Primary navigation (these are the ones we should use going forward)
  NavNext,
  NavPrev,
  Select,
  Back,
  Home,

  // Backward-compat aliases (keep existing code compiling while we migrate)
  Next = NavNext,
  Prev = NavPrev,
};
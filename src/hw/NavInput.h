// /src/hw/NavInput.h
#pragma once

#include "UiInput.h"

// NavInput is the "app-facing" name for UI navigation events.
// For now it's just an alias to UiInput (small event queue).
// Later we can replace it with something else if needed.
using NavInput = UiInput;
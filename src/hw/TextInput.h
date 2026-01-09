// /src/hw/TextInput.h
#pragma once

#include "Input.h"

// TextInput is the "app-facing" name for text entry.
// For now it's just an alias to Input (line-based via serial).
// Later we can implement a keyboard-based version (I2C, etc.).
using TextInput = Input;
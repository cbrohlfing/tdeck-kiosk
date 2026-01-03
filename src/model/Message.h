#pragma once
#include <Arduino.h>

struct Message {
  String peer;     // conversation id (e.g., "bob")
  String text;
  bool inbound;    // true = received, false = sent
  uint32_t tsMs;   // millis() timestamp for now
};
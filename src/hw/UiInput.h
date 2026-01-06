#pragma once
#include "UiInputEvent.h"

class UiInput {
public:
  virtual ~UiInput() = default;
  virtual void begin() {}
  virtual UiInputEvent pollEvent() { return UiInputEvent::NONE; }
};

// Default no-op implementation so boards without UI input still compile cleanly.
class NullUiInput : public UiInput {};
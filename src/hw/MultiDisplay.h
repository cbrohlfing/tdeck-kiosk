#pragma once
#include "Display.h"

class MultiDisplay : public Display {
public:
  MultiDisplay(Display* a, Display* b) : _a(a), _b(b) {}

  void clear() override {
    if (_a) _a->clear();
    if (_b) _b->clear();
  }

  void line(const String& s) override {
    if (_a) _a->line(s);
    if (_b) _b->line(s);
  }

private:
  Display* _a = nullptr;
  Display* _b = nullptr;
};
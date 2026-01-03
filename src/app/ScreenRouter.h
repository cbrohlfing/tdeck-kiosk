#pragma once
#include <Arduino.h>
#include "Screens.h"

class LockManager;
class Display;
class Input;

class ScreenRouter {
public:
  void begin(LockManager* lock, Display* display, Input* input);
  void tick();

private:
  LockManager* lockMgr = nullptr;
  Display* ui = nullptr;
  Input* in = nullptr;

  ScreenId current = ScreenId::KidHome;

  // Unlock screen state
  bool unlockError = false;

  void go(ScreenId next);
  void render();

  void handleLine(const String& line);
};
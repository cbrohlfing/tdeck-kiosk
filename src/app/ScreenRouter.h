#pragma once
#include "Screens.h"

class LockManager;

class ScreenRouter {
public:
  void begin(LockManager* lock);
  void go(ScreenId next);
  void back();
  void tick(); // called from loop()

private:
  LockManager* lockMgr = nullptr;
  ScreenId current = ScreenId::KidHome;
  ScreenId previous = ScreenId::KidHome;

  void render();
};
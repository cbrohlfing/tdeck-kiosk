#pragma once
#include <stdbool.h>

class LockManager {
public:
  void begin();

  bool isKidMode() const;
  bool verifyPin(const char* pin);
  void unlockAdmin();
  void lockToKid();

private:
  bool kidMode = true;
};
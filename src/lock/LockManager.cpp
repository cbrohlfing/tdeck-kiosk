#include "LockManager.h"
#include <Arduino.h>
#include <Preferences.h>

static Preferences prefs;
static const char* PREF_NS = "lock";

void LockManager::begin() {
  prefs.begin(PREF_NS, false);
  kidMode = prefs.getBool("kid", true);
}

bool LockManager::isKidMode() const {
  return kidMode;
}

bool LockManager::verifyPin(const char* pin) {
  // TEMP: hardcoded PIN for bring-up
  if (strcmp(pin, "1234") == 0) {
    unlockAdmin();
    return true;
  }
  return false;
}

void LockManager::unlockAdmin() {
  kidMode = false;
}

void LockManager::lockToKid() {
  kidMode = true;
}
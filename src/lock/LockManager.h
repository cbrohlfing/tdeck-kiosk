#pragma once
#include <Arduino.h>

class LockManager {
public:
  void begin();

  bool isKidMode() const;

  // Unlock flow
  bool verifyPin(const String& pin);
  void unlockAdmin();
  void lockToKid();

  // Admin actions
  bool setPin(const String& newPin);
  void setKidDefault(bool enabled);
  bool getKidDefault() const;

  // Lockout status
  bool isLockedOut() const;
  uint32_t lockoutRemainingSeconds() const;

private:
  bool kidMode = true;
  bool kidDefault = true;

  // Security / lockout
  uint8_t failedAttempts = 0;
  uint32_t lockoutUntilMs = 0;

  // Stored credentials
  bool hasCreds = false;

  void ensureCredsExist();
  bool isValidPinFormat(const String& pin) const;
};
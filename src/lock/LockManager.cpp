#include "LockManager.h"
#include <Preferences.h>
#include <mbedtls/sha256.h>
#include <esp_system.h>

static Preferences prefs;
static const char* PREF_NS = "lock";

// NVS keys
static const char* KEY_KID_DEFAULT = "kid_default";
static const char* KEY_SALT = "pin_salt";   // bytes
static const char* KEY_HASH = "pin_hash";   // bytes

// Policy
static const uint8_t MAX_FAILS = 5;
static const uint32_t LOCKOUT_MS = 30 * 1000; // 30 seconds

static void sha256(const uint8_t* data, size_t len, uint8_t out[32]) {
  mbedtls_sha256_context ctx;
  mbedtls_sha256_init(&ctx);
  mbedtls_sha256_starts_ret(&ctx, 0 /*is224*/);
  mbedtls_sha256_update_ret(&ctx, data, len);
  mbedtls_sha256_finish_ret(&ctx, out);
  mbedtls_sha256_free(&ctx);
}

static void computePinHash(const uint8_t salt[16], const String& pin, uint8_t outHash[32]) {
  // hash = SHA256(salt || pin)
  uint8_t buf[16 + 16]; // 16 salt + up to 16 pin chars (we enforce 4-6 digits)
  size_t pinLen = (size_t)pin.length();
  if (pinLen > 16) pinLen = 16;

  memcpy(buf, salt, 16);
  memcpy(buf + 16, pin.c_str(), pinLen);
  sha256(buf, 16 + pinLen, outHash);
}

bool LockManager::isValidPinFormat(const String& pin) const {
  if (pin.length() < 4 || pin.length() > 6) return false;
  for (size_t i = 0; i < pin.length(); i++) {
    if (pin[i] < '0' || pin[i] > '9') return false;
  }
  return true;
}

void LockManager::begin() {
  prefs.begin(PREF_NS, false);

  kidDefault = prefs.getBool(KEY_KID_DEFAULT, true);
  kidMode = kidDefault;

  ensureCredsExist();
}

void LockManager::ensureCredsExist() {
  size_t saltLen = prefs.getBytesLength(KEY_SALT);
  size_t hashLen = prefs.getBytesLength(KEY_HASH);

  if (saltLen == 16 && hashLen == 32) {
    hasCreds = true;
    return;
  }

  // Create default creds (PIN 1234) for bring-up; user should change in admin
  uint8_t salt[16];
  for (int i = 0; i < 16; i++) salt[i] = (uint8_t)(esp_random() & 0xFF);

  uint8_t hash[32];
  computePinHash(salt, "1234", hash);

  prefs.putBytes(KEY_SALT, salt, 16);
  prefs.putBytes(KEY_HASH, hash, 32);
  hasCreds = true;
}

bool LockManager::isKidMode() const {
  return kidMode;
}

bool LockManager::isLockedOut() const {
  return lockoutUntilMs != 0 && (int32_t)(lockoutUntilMs - millis()) > 0;
}

uint32_t LockManager::lockoutRemainingSeconds() const {
  if (!isLockedOut()) return 0;
  uint32_t remainingMs = lockoutUntilMs - millis();
  return (remainingMs + 999) / 1000;
}

bool LockManager::verifyPin(const String& pin) {
  if (isLockedOut()) return false;
  if (!hasCreds) return false;
  if (!isValidPinFormat(pin)) return false;

  uint8_t salt[16];
  uint8_t storedHash[32];
  uint8_t calcHash[32];

  if (prefs.getBytes(KEY_SALT, salt, 16) != 16) return false;
  if (prefs.getBytes(KEY_HASH, storedHash, 32) != 32) return false;

  computePinHash(salt, pin, calcHash);

  bool ok = (memcmp(storedHash, calcHash, 32) == 0);

  if (ok) {
    failedAttempts = 0;
    unlockAdmin();
    return true;
  }

  failedAttempts++;
  if (failedAttempts >= MAX_FAILS) {
    lockoutUntilMs = millis() + LOCKOUT_MS;
    failedAttempts = 0;
  }
  return false;
}

void LockManager::unlockAdmin() {
  kidMode = false;
}

void LockManager::lockToKid() {
  kidMode = true;
}

bool LockManager::setPin(const String& newPin) {
  if (!isValidPinFormat(newPin)) return false;

  uint8_t salt[16];
  for (int i = 0; i < 16; i++) salt[i] = (uint8_t)(esp_random() & 0xFF);

  uint8_t hash[32];
  computePinHash(salt, newPin, hash);

  prefs.putBytes(KEY_SALT, salt, 16);
  prefs.putBytes(KEY_HASH, hash, 32);
  hasCreds = true;
  return true;
}

void LockManager::setKidDefault(bool enabled) {
  kidDefault = enabled;
  prefs.putBool(KEY_KID_DEFAULT, enabled);
}

bool LockManager::getKidDefault() const {
  return kidDefault;
}
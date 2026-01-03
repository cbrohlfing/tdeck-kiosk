#include "ScreenRouter.h"
#include "../lock/LockManager.h"
#include <Arduino.h>

static const char* screenName(ScreenId s) {
  switch (s) {
    case ScreenId::KidHome:   return "KidHome";
    case ScreenId::Inbox:     return "Inbox";
    case ScreenId::Unlock:    return "Unlock";
    case ScreenId::AdminHome: return "AdminHome";
    default: return "Unknown";
  }
}

void ScreenRouter::begin(LockManager* lock) {
  lockMgr = lock;
  current = ScreenId::KidHome;
  previous = ScreenId::KidHome;
  render();
}

void ScreenRouter::go(ScreenId next) {
  // Gate screens in Kid Mode
  if (lockMgr && lockMgr->isKidMode()) {
    if (!(next == ScreenId::KidHome || next == ScreenId::Inbox || next == ScreenId::Unlock)) {
      Serial.println("[router] Blocked by Kid Mode");
      return;
    }
  }

  previous = current;
  current = next;
  render();
}

void ScreenRouter::back() {
  ScreenId tmp = current;
  current = previous;
  previous = tmp;
  render();
}

void ScreenRouter::tick() {
  // For now, “demo navigation” every few seconds so you can see the flow
  static unsigned long last = 0;
  static int step = 0;

  if (millis() - last < 2500) return;
  last = millis();

  if (lockMgr && lockMgr->isKidMode()) {
    // Kid flow: KidHome -> Inbox -> Unlock -> (try AdminHome blocked unless unlocked)
    if (step == 0) go(ScreenId::Inbox);
    else if (step == 1) go(ScreenId::Unlock);
    else if (step == 2) {
      Serial.println("[unlock] Trying PIN 1234...");
      if (lockMgr->verifyPin("1234")) Serial.println("[unlock] Success. Now in ADMIN mode");
      go(ScreenId::AdminHome);
    } else {
      step = -1;
      go(ScreenId::KidHome);
    }
  } else {
    // Admin flow: AdminHome -> KidHome -> (lock) -> KidHome
    if (step == 0) go(ScreenId::AdminHome);
    else if (step == 1) {
      Serial.println("[admin] Locking back to Kid Mode");
      lockMgr->lockToKid();
      go(ScreenId::KidHome);
    } else {
      step = -1;
      go(ScreenId::KidHome);
    }
  }

  step++;
}

void ScreenRouter::render() {
  Serial.print("[screen] ");
  Serial.print(screenName(current));
  Serial.print(" | mode=");
  Serial.println(lockMgr && lockMgr->isKidMode() ? "KID" : "ADMIN");
}
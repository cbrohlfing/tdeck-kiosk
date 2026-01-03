#include "ScreenRouter.h"
#include "../lock/LockManager.h"
#include "../hw/Display.h"
#include "../hw/Input.h"

static const char* screenName(ScreenId s) {
  switch (s) {
    case ScreenId::KidHome:   return "KidHome";
    case ScreenId::Inbox:     return "Inbox";
    case ScreenId::Unlock:    return "Unlock";
    case ScreenId::AdminHome: return "AdminHome";
    default: return "Unknown";
  }
}

void ScreenRouter::begin(LockManager* lock, Display* display, Input* input) {
  lockMgr = lock;
  ui = display;
  in = input;
  current = ScreenId::KidHome;
  render();
}

void ScreenRouter::go(ScreenId next) {
  // Enforce Kid Mode route gating
  if (lockMgr && lockMgr->isKidMode()) {
    if (!(next == ScreenId::KidHome || next == ScreenId::Inbox || next == ScreenId::Unlock)) {
      ui->line("[router] Blocked by Kid Mode");
      return;
    }
  }

  current = next;
  unlockError = false;
  render();
}

void ScreenRouter::tick() {
  String line;
  if (in && in->pollLine(line)) {
    handleLine(line);
  }
}

void ScreenRouter::handleLine(const String& line) {
  if (!ui || !lockMgr) return;

  if (line.length() == 0) return;

  // Global helpers
  if (line == "help") {
    ui->line("Type the command shown in brackets (e.g., i, a, back).");
    ui->line("Unlock screen: type your PIN and press Enter.");
    return;
  }

  if (line == "back") {
    go(lockMgr->isKidMode() ? ScreenId::KidHome : ScreenId::AdminHome);
    return;
  }

  switch (current) {
    case ScreenId::KidHome:
      if (line == "i") go(ScreenId::Inbox);
      else if (line == "a") go(ScreenId::Unlock);
      else ui->line("Unknown. Try: i, a, help");
      break;

    case ScreenId::Inbox:
      if (line == "h") go(ScreenId::KidHome);
      else ui->line("Unknown. Try: h");
      break;

    case ScreenId::Unlock:
      if (lockMgr->isLockedOut()) {
        ui->line("Locked out. Wait " + String(lockMgr->lockoutRemainingSeconds()) + "s.");
        return;
      }
      if (line == "x") {
        go(ScreenId::KidHome);
        return;
      }

      if (lockMgr->verifyPin(line)) {
        ui->line("✅ Unlocked. Welcome, Admin.");
        go(ScreenId::AdminHome);
      } else {
        unlockError = true;
        render();
      }
      break;

    case ScreenId::AdminHome:
      if (line == "k") { lockMgr->lockToKid(); go(ScreenId::KidHome); }
      else if (line == "pin") {
        ui->line("Enter new PIN (4-6 digits):");
        // Simple one-step change: next line sets the pin
        current = ScreenId::AdminHome; // stay logically here
        // Reuse Unlock screen behavior? Keep it simple: treat next input line as pin if prefixed
        ui->line("Type: setpin 5555");
      }
      else if (line.startsWith("setpin ")) {
        String p = line.substring(7);
        p.trim();
        if (lockMgr->setPin(p)) ui->line("✅ PIN updated.");
        else ui->line("❌ Invalid PIN. Use 4-6 digits.");
      }
      else if (line == "kiddefault on") { lockMgr->setKidDefault(true); ui->line("Kid default ON"); }
      else if (line == "kiddefault off") { lockMgr->setKidDefault(false); ui->line("Kid default OFF"); }
      else ui->line("Unknown. Try: k, setpin 5555, kiddefault on/off");
      break;
  }
}

void ScreenRouter::render() {
  ui->clear();
  ui->line(String("[screen] ") + screenName(current) + " | mode=" + (lockMgr->isKidMode() ? "KID" : "ADMIN"));
  ui->line("");

  switch (current) {
    case ScreenId::KidHome:
      ui->line("Kid Mode Home");
      ui->line("Commands:");
      ui->line("  [i] Inbox");
      ui->line("  [a] Admin (unlock)");
      ui->line("  help");
      break;

    case ScreenId::Inbox:
      ui->line("Inbox (stub)");
      ui->line("Commands:");
      ui->line("  [h] Home");
      break;

    case ScreenId::Unlock:
      ui->line("Admin Unlock");
      if (lockMgr->isLockedOut()) {
        ui->line("LOCKED OUT. Wait " + String(lockMgr->lockoutRemainingSeconds()) + "s.");
        ui->line("Type: back");
      } else {
        if (unlockError) ui->line("❌ Wrong PIN. Try again.");
        ui->line("Enter PIN (4-6 digits) then Enter.");
        ui->line("Commands:");
        ui->line("  [x] Cancel");
      }
      break;

    case ScreenId::AdminHome:
      ui->line("Admin Home");
      ui->line("Commands:");
      ui->line("  [k] Lock to Kid Mode");
      ui->line("  setpin 5555");
      ui->line("  kiddefault on");
      ui->line("  kiddefault off");
      break;
  }
}
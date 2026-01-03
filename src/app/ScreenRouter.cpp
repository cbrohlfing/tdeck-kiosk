#include "ScreenRouter.h"
#include "../lock/LockManager.h"
#include "../hw/Display.h"
#include "../hw/Input.h"
#include "../mesh/MeshService.h"
#include "../model/MessageStore.h"
#include "../model/Message.h"

static const char* screenName(ScreenId s) {
  switch (s) {
    case ScreenId::KidHome:   return "KidHome";
    case ScreenId::Inbox:     return "Inbox";
    case ScreenId::Chat:      return "Chat";
    case ScreenId::Unlock:    return "Unlock";
    case ScreenId::AdminHome: return "AdminHome";
    default: return "Unknown";
  }
}

void ScreenRouter::begin(LockManager* lock, Display* display, Input* input, MeshService* mesh, MessageStore* store) {
  lockMgr = lock;
  ui = display;
  in = input;
  meshSvc = mesh;
  msgStore = store;

  current = ScreenId::KidHome;
  render();
}

void ScreenRouter::go(ScreenId next) {
  // Enforce Kid Mode route gating
  if (lockMgr && lockMgr->isKidMode()) {
    if (!(next == ScreenId::KidHome || next == ScreenId::Inbox || next == ScreenId::Chat || next == ScreenId::Unlock)) {
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
    if (line.length() == 0) return;
    handleLine(line);
  }
}

static bool split2(const String& s, char delim, String& left, String& right) {
  int idx = s.indexOf(delim);
  if (idx < 0) return false;
  left = s.substring(0, idx);
  right = s.substring(idx + 1);
  left.trim();
  right.trim();
  return true;
}

void ScreenRouter::handleLine(const String& line) {
  if (!ui || !lockMgr) return;

  // Global help
  if (line == "help") {
    ui->line("Commands:");
    ui->line("  home | inbox | chat <peer> | send <peer> <msg> | rx <peer> <msg>");
    ui->line("  back");
    return;
  }

  if (line == "home") { go(ScreenId::KidHome); return; }
  if (line == "inbox") { go(ScreenId::Inbox); return; }

  if (line == "back") {
    if (lockMgr->isKidMode()) go(ScreenId::KidHome);
    else go(ScreenId::AdminHome);
    return;
  }

  // chat <peer>
  if (line.startsWith("chat ")) {
    activePeer = line.substring(5);
    activePeer.trim();
    if (activePeer.length() == 0) { ui->line("Usage: chat <peer>"); return; }
    go(ScreenId::Chat);
    return;
  }

  // send <peer> <msg>
  if (line.startsWith("send ")) {
    String rest = line.substring(5);
    rest.trim();
    String peer, msg;
    if (!split2(rest, ' ', peer, msg) || peer.length() == 0 || msg.length() == 0) {
      ui->line("Usage: send <peer> <message>");
      return;
    }

    if (!meshSvc) { ui->line("Mesh not ready"); return; }

    bool ok = meshSvc->send(peer, msg);
    if (ok && msgStore) {
      Message m{peer, msg, false, millis()};
      msgStore->add(m);
      ui->line("✅ Sent.");
      activePeer = peer;
      go(ScreenId::Chat);
    } else {
      ui->line("❌ Send failed.");
    }
    return;
  }

  // rx <peer> <msg>  (simulate inbound)
  if (line.startsWith("rx ")) {
    String rest = line.substring(3);
    rest.trim();
    String peer, msg;
    if (!split2(rest, ' ', peer, msg) || peer.length() == 0 || msg.length() == 0) {
      ui->line("Usage: rx <peer> <message>");
      return;
    }
    if (meshSvc) meshSvc->injectInbound(peer, msg);
    ui->line("📥 Inbound injected.");
    return;
  }

  // Per-screen controls
  switch (current) {
    case ScreenId::KidHome:
      if (line == "a") go(ScreenId::Unlock);
      else ui->line("Try: inbox | chat <peer> | send <peer> <msg> | a | help");
      break;

    case ScreenId::Inbox:
      // Shortcut: enter a peer name to open chat
      activePeer = line;
      activePeer.trim();
      if (activePeer.length() == 0) return;
      go(ScreenId::Chat);
      break;

    case ScreenId::Chat:
      // In chat screen: typing sends to active peer
      if (activePeer.length() == 0) { ui->line("No active peer. Use: chat <peer>"); return; }
      if (meshSvc && msgStore) {
        bool ok = meshSvc->send(activePeer, line);
        if (ok) {
          msgStore->add(Message{activePeer, line, false, millis()});
          render();
        } else ui->line("❌ Send failed.");
      }
      break;

    case ScreenId::Unlock:
      if (lockMgr->isLockedOut()) { render(); return; }
      if (line == "x") { go(ScreenId::KidHome); return; }

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
      else if (line.startsWith("setpin ")) {
        String p = line.substring(7);
        p.trim();
        if (lockMgr->setPin(p)) ui->line("✅ PIN updated.");
        else ui->line("❌ Invalid PIN. Use 4-6 digits.");
      }
      else if (line == "kiddefault on") { lockMgr->setKidDefault(true); ui->line("Kid default ON"); }
      else if (line == "kiddefault off") { lockMgr->setKidDefault(false); ui->line("Kid default OFF"); }
      else ui->line("Try: k | setpin 5555 | kiddefault on/off | help");
      break;
  }
}

void ScreenRouter::renderInbox() {
  ui->line("Inbox (peers):");
  if (!msgStore) { ui->line("  (no store)"); return; }
  auto peers = msgStore->peersMostRecentFirst();
  if (peers.empty()) {
    ui->line("  (empty) - try: rx bob hello");
    return;
  }
  for (auto& p : peers) {
    ui->line("  - " + p + "   (type name to open chat)");
  }
}

void ScreenRouter::renderChat(const String& peer) {
  ui->line("Chat: " + peer);
  if (!msgStore) { ui->line("  (no store)"); return; }
  auto msgs = msgStore->messagesFor(peer);
  if (msgs.empty()) {
    ui->line("  (no messages) - type a message to send");
    return;
  }
  // Show last ~10
  int start = (int)msgs.size() - 10;
  if (start < 0) start = 0;
  for (int i = start; i < (int)msgs.size(); i++) {
    const auto& m = msgs[i];
    ui->line(String(m.inbound ? "< " : "> ") + m.text);
  }
  ui->line("");
  ui->line("Type to send. (or: back | inbox | home)");
}

void ScreenRouter::render() {
  ui->clear();
  ui->line(String("[screen] ") + screenName(current) + " | mode=" + (lockMgr->isKidMode() ? "KID" : "ADMIN"));
  ui->line("");

  switch (current) {
    case ScreenId::KidHome:
      ui->line("Kid Mode Home");
      ui->line("Try:");
      ui->line("  inbox");
      ui->line("  chat bob");
      ui->line("  send bob hi there");
      ui->line("  rx bob hi (simulate inbound)");
      ui->line("  a (admin unlock)");
      ui->line("  help");
      break;

    case ScreenId::Inbox:
      renderInbox();
      ui->line("");
      ui->line("Commands: home | chat <peer> | back");
      break;

    case ScreenId::Chat:
      if (activePeer.length() == 0) ui->line("No active peer. Use: chat <peer>");
      else renderChat(activePeer);
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
      ui->line("  help");
      break;
  }
}
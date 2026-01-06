// /src/app/ScreenRouter.cpp
#include "ScreenRouter.h"

#include "../lock/LockManager.h"
#include "../hw/Display.h"
#include "../hw/Input.h"
#include "../hw/UiInput.h"
#include "../hw/UiInputEvent.h"
#include "../mesh/MeshService.h"
#include "../model/MessageStore.h"
#include "../model/Message.h"

static constexpr uint8_t kDisplayLines = 4;
static constexpr uint8_t kHeaderLines  = 1;
static constexpr uint8_t kBodyLines    = (kDisplayLines - kHeaderLines);

static constexpr uint8_t kChatMsgLinesPerPage = 2;

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

static bool split2(const String& s, char delim, String& left, String& right) {
  int idx = s.indexOf(delim);
  if (idx < 0) return false;
  left = s.substring(0, idx);
  right = s.substring(idx + 1);
  left.trim();
  right.trim();
  return true;
}

static String clip(const String& s, uint16_t maxLen) {
  if (s.length() <= maxLen) return s;
  return s.substring(0, maxLen - 1) + "~";
}

static void clampListWindow(uint16_t totalItems, uint16_t& cursor, uint16_t& top) {
  if (totalItems == 0) { cursor = 0; top = 0; return; }
  if (cursor >= totalItems) cursor = 0;

  if (cursor < top) top = cursor;
  const uint16_t window = kBodyLines;
  if (cursor >= (uint16_t)(top + window)) {
    top = cursor - (window - 1);
  }

  if (top >= totalItems) top = 0;
}

void ScreenRouter::begin(LockManager* lock,
                         Display* display,
                         Input* input,
                         UiInput* uiInput,
                         MeshService* mesh,
                         MessageStore* store) {
  lockMgr = lock;
  ui = display;
  in = input;
  uiIn = uiInput;
  meshSvc = mesh;
  msgStore = store;

  current = ScreenId::KidHome;
  resetUiState();
  render();
}

void ScreenRouter::go(ScreenId next) {
  if (lockMgr && lockMgr->isKidMode()) {
    if (!(next == ScreenId::KidHome || next == ScreenId::Inbox || next == ScreenId::Chat || next == ScreenId::Unlock)) {
      if (ui) ui->line("[router] Blocked");
      return;
    }
  }

  current = next;
  unlockError = false;
  resetUiState();
  render();
}

void ScreenRouter::tick() {
  if (uiIn) {
    UiInputEvent e;
    while (uiIn->take(e)) {
      handleUiEvent((uint8_t)e);
    }
  }

  String line;
  if (in && in->pollLine(line)) {
    line.trim();
    if (line.length() == 0) return;
    handleLine(line);
  }
}

void ScreenRouter::handleUiEvent(uint8_t eRaw) {
  UiInputEvent e = (UiInputEvent)eRaw;

  switch (e) {
    case UiInputEvent::NavNext: uiNext();   break;
    case UiInputEvent::NavPrev: uiPrev();   break;
    case UiInputEvent::Select:  uiSelect(); break;
    case UiInputEvent::Back:    uiBack();   break;
    case UiInputEvent::Home:    uiHome();   break;
    default: break;
  }
}

void ScreenRouter::uiNext() {
  if (!ui || !lockMgr) return;

  switch (current) {
    case ScreenId::KidHome: {
      cursor = (cursor + 1) % 3;
      render();
      break;
    }

    case ScreenId::Inbox: {
      uint16_t peersCount = 0;
      if (msgStore) peersCount = (uint16_t)msgStore->peersMostRecentFirst().size();
      const uint16_t total = peersCount + 1;

      cursor = (total == 0) ? 0 : (uint16_t)((cursor + 1) % total);
      clampListWindow(total, cursor, listTop);
      render();
      break;
    }

    case ScreenId::Chat: {
      if (!msgStore || activePeer.length() == 0) { render(); return; }
      auto msgs = msgStore->messagesFor(activePeer);
      uint16_t total = (uint16_t)msgs.size();
      if (total == 0) { render(); return; }

      uint16_t pages = (total + kChatMsgLinesPerPage - 1) / kChatMsgLinesPerPage;
      if (pages == 0) pages = 1;

      page = (page + 1) % pages;
      render();
      break;
    }

    case ScreenId::Unlock: {
      render();
      break;
    }

    case ScreenId::AdminHome: {
      cursor = (cursor + 1) % 2;
      render();
      break;
    }
  }
}

void ScreenRouter::uiPrev() {
  if (!ui || !lockMgr) return;

  switch (current) {
    case ScreenId::KidHome: {
      cursor = (cursor + 2) % 3;
      render();
      break;
    }

    case ScreenId::Inbox: {
      uint16_t peersCount = 0;
      if (msgStore) peersCount = (uint16_t)msgStore->peersMostRecentFirst().size();
      const uint16_t total = peersCount + 1;

      if (total == 0) { cursor = 0; listTop = 0; render(); return; }
      cursor = (cursor == 0) ? (uint16_t)(total - 1) : (uint16_t)(cursor - 1);
      clampListWindow(total, cursor, listTop);
      render();
      break;
    }

    case ScreenId::Chat: {
      if (!msgStore || activePeer.length() == 0) { render(); return; }
      auto msgs = msgStore->messagesFor(activePeer);
      uint16_t total = (uint16_t)msgs.size();
      if (total == 0) { render(); return; }

      uint16_t pages = (total + kChatMsgLinesPerPage - 1) / kChatMsgLinesPerPage;
      if (pages == 0) pages = 1;

      page = (page == 0) ? (uint16_t)(pages - 1) : (uint16_t)(page - 1);
      render();
      break;
    }

    case ScreenId::Unlock: {
      render();
      break;
    }

    case ScreenId::AdminHome: {
      cursor = (cursor + 1) % 2;
      render();
      break;
    }
  }
}

void ScreenRouter::uiSelect() {
  if (!ui || !lockMgr) return;

  switch (current) {
    case ScreenId::KidHome: {
      if (cursor == 0) { go(ScreenId::Inbox); return; }

      if (cursor == 1) {
        if (msgStore) {
          auto peers = msgStore->peersMostRecentFirst();
          if (!peers.empty()) {
            activePeer = peers[0];
            go(ScreenId::Chat);
            return;
          }
        }
        go(ScreenId::Inbox);
        return;
      }

      if (cursor == 2) { go(ScreenId::Unlock); return; }
      break;
    }

    case ScreenId::Inbox: {
      if (!msgStore) { go(ScreenId::KidHome); return; }

      auto peers = msgStore->peersMostRecentFirst();
      const uint16_t peersCount = (uint16_t)peers.size();
      const uint16_t homeIndex = peersCount;

      if (cursor == homeIndex) {
        go(ScreenId::KidHome);
        return;
      }

      if (peersCount == 0) {
        go(ScreenId::KidHome);
        return;
      }

      if (cursor >= peersCount) cursor = 0;
      activePeer = peers[cursor];
      go(ScreenId::Chat);
      return;
    }

    case ScreenId::Chat: {
      go(ScreenId::Inbox);
      return;
    }

    case ScreenId::Unlock: {
      go(ScreenId::KidHome);
      return;
    }

    case ScreenId::AdminHome: {
      if (cursor == 0) {
        lockMgr->lockToKid();
        go(ScreenId::KidHome);
        return;
      }
      if (cursor == 1) {
        go(ScreenId::Inbox);
        return;
      }
      break;
    }
  }
}

void ScreenRouter::uiBack() {
  if (!lockMgr) return;

  switch (current) {
    case ScreenId::KidHome:
      return;

    case ScreenId::Inbox:
      if (lockMgr->isKidMode()) go(ScreenId::KidHome);
      else go(ScreenId::AdminHome);
      return;

    case ScreenId::Chat:
      go(ScreenId::Inbox);
      return;

    case ScreenId::Unlock:
      go(ScreenId::KidHome);
      return;

    case ScreenId::AdminHome:
      go(ScreenId::KidHome);
      return;
  }
}

void ScreenRouter::uiHome() {
  go(ScreenId::KidHome);
}

void ScreenRouter::handleLine(const String& lineIn) {
  if (!ui || !lockMgr) return;

  String line = lineIn;
  line.trim();

  if (line == "home")  { go(ScreenId::KidHome); return; }
  if (line == "inbox") { go(ScreenId::Inbox);   return; }

  if (line == "back") {
    if (lockMgr->isKidMode()) go(ScreenId::KidHome);
    else go(ScreenId::AdminHome);
    return;
  }

  if (line == "help") {
    ui->line("home/inbox/back");
    ui->line("chat <peer>");
    ui->line("send <p> <m>");
    ui->line("rx <p> <m>");
    return;
  }

  if (line.startsWith("mc ")) {
    String cmd = line.substring(3);
    cmd.trim();
    if (cmd.length() == 0) { ui->line("mc <cmd>"); return; }
    if (!meshSvc) { ui->line("mesh n/a"); return; }

    String out;
    bool ok = meshSvc->backendCli(cmd, &out);
    if (ok) {
      if (out.length() > 0) ui->line(out);
    } else {
      ui->line("mc rejected");
    }
    return;
  }

  if (line.startsWith("chat ")) {
    activePeer = line.substring(5);
    activePeer.trim();
    if (activePeer.length() == 0) { ui->line("chat <peer>"); return; }
    go(ScreenId::Chat);
    return;
  }

  if (line.startsWith("send ")) {
    String rest = line.substring(5);
    rest.trim();
    String peer, msg;
    if (!split2(rest, ' ', peer, msg) || peer.length() == 0 || msg.length() == 0) {
      ui->line("send <p> <m>");
      return;
    }

    if (!meshSvc) { ui->line("mesh n/a"); return; }

    bool ok = meshSvc->send(peer, msg);
    if (ok && msgStore) {
      msgStore->add(Message{peer, msg, false, millis()});
      activePeer = peer;
      go(ScreenId::Chat);
    } else {
      ui->line("send fail");
    }
    return;
  }

  if (line.startsWith("rx ")) {
    String rest = line.substring(3);
    rest.trim();
    String peer, msg;
    if (!split2(rest, ' ', peer, msg) || peer.length() == 0 || msg.length() == 0) {
      ui->line("rx <p> <m>");
      return;
    }

    if (msgStore) msgStore->add(Message{peer, msg, true, millis()});
    if (meshSvc) meshSvc->injectInbound(peer, msg);

    if (current == ScreenId::Inbox) render();
    else if (current == ScreenId::Chat && activePeer == peer) render();
    else ui->line("rx ok");
    return;
  }

  switch (current) {
    case ScreenId::KidHome:
      if (line == "a") go(ScreenId::Unlock);
      else ui->line("home/inbox/chat");
      break;

    case ScreenId::Inbox:
      activePeer = line;
      activePeer.trim();
      if (activePeer.length() == 0) return;
      go(ScreenId::Chat);
      break;

    case ScreenId::Chat:
      if (activePeer.length() == 0) { ui->line("chat <peer>"); return; }
      if (meshSvc && msgStore) {
        bool ok = meshSvc->send(activePeer, line);
        if (ok) {
          msgStore->add(Message{activePeer, line, false, millis()});
          render();
        } else {
          ui->line("send fail");
        }
      }
      break;

    case ScreenId::Unlock:
      if (lockMgr->isLockedOut()) { render(); return; }
      if (line == "x") { go(ScreenId::KidHome); return; }

      if (lockMgr->verifyPin(line)) {
        go(ScreenId::AdminHome);
      } else {
        unlockError = true;
        render();
      }
      break;

    case ScreenId::AdminHome:
      if (line == "k") { lockMgr->lockToKid(); go(ScreenId::KidHome); }
      else ui->line("k / inbox");
      break;
  }
}

void ScreenRouter::renderInbox() {
  if (!ui) return;

  ui->clear();
  ui->line("Inbox");

  if (!msgStore) {
    ui->line("> Home");
    ui->line("");
    ui->line("");
    return;
  }

  auto peers = msgStore->peersMostRecentFirst();
  const uint16_t peersCount = (uint16_t)peers.size();
  const uint16_t totalItems = peersCount + 1;

  clampListWindow(totalItems, cursor, listTop);

  for (uint8_t row = 0; row < kBodyLines; row++) {
    uint16_t idx = listTop + row;
    if (idx >= totalItems) {
      ui->line("");
      continue;
    }

    const bool sel = (idx == cursor);
    String label;

    if (idx < peersCount) {
      label = clip(peers[idx], 18);
    } else {
      label = "Home";
    }

    ui->line(String(sel ? "> " : "  ") + label);
  }
}

void ScreenRouter::renderChat(const String& peer) {
  if (!ui) return;

  ui->clear();
  ui->line("Chat " + clip(peer, 12));

  if (!msgStore) {
    ui->line("(no store)");
    ui->line("");
    ui->line("Long=Back");
    return;
  }

  auto msgs = msgStore->messagesFor(peer);
  const uint16_t total = (uint16_t)msgs.size();
  if (total == 0) {
    ui->line("(empty)");
    ui->line("");
    ui->line("Long=Back");
    return;
  }

  uint16_t pages = (total + kChatMsgLinesPerPage - 1) / kChatMsgLinesPerPage;
  if (pages == 0) pages = 1;
  if (page >= pages) page = 0;

  int end = (int)total - (int)(page * kChatMsgLinesPerPage);
  int start = end - (int)kChatMsgLinesPerPage;
  if (start < 0) start = 0;
  if (end < 0) end = 0;

  for (int i = start; i < end; i++) {
    const auto& m = msgs[i];
    String t = clip(m.text, 18);
    ui->line(String(m.inbound ? "<" : ">") + t);
  }
  if ((end - start) < kChatMsgLinesPerPage) ui->line("");

  ui->line("Pg " + String(page + 1) + "/" + String(pages));
}

void ScreenRouter::render() {
  if (!ui || !lockMgr) return;

  switch (current) {
    case ScreenId::KidHome: {
      ui->clear();
      ui->line("Kid Home");
      ui->line(String(cursor == 0 ? "> " : "  ") + "Inbox");
      ui->line(String(cursor == 1 ? "> " : "  ") + "Chat");
      ui->line(String(cursor == 2 ? "> " : "  ") + "Unlock");
      break;
    }

    case ScreenId::Inbox:
      renderInbox();
      break;

    case ScreenId::Chat:
      if (activePeer.length() == 0) {
        ui->clear();
        ui->line("Chat");
        ui->line("Use serial:");
        ui->line("chat <peer>");
        ui->line("Long=Back");
      } else {
        renderChat(activePeer);
      }
      break;

    case ScreenId::Unlock: {
      ui->clear();
      ui->line("Unlock");
      if (lockMgr->isLockedOut()) {
        ui->line("LOCKOUT");
        ui->line(String(lockMgr->lockoutRemainingSeconds()) + "s left");
        ui->line("Long=Home");
      } else {
        ui->line(unlockError ? "Bad PIN" : "Enter PIN");
        ui->line("via serial");
        ui->line("Long=Home");
      }
      break;
    }

    case ScreenId::AdminHome: {
      ui->clear();
      ui->line("Admin");
      ui->line(String(cursor == 0 ? "> " : "  ") + "Lock Kid");
      ui->line(String(cursor == 1 ? "> " : "  ") + "Inbox");
      ui->line("Long=Select");
      break;
    }
  }
}
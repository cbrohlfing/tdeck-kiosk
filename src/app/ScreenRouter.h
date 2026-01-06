// /src/app/ScreenRouter.h
#pragma once
#include <Arduino.h>

class LockManager;
class Display;
class Input;
class UiInput;
class MeshService;
class MessageStore;

enum class ScreenId : uint8_t {
  KidHome = 0,
  Inbox,
  Chat,
  Unlock,
  AdminHome,
};

class ScreenRouter {
public:
  void begin(LockManager* lock,
             Display* display,
             Input* input,
             UiInput* uiInput,
             MeshService* mesh,
             MessageStore* store);

  void tick();

private:
  // Services
  LockManager* lockMgr = nullptr;
  Display* ui = nullptr;
  Input* in = nullptr;
  UiInput* uiIn = nullptr;
  MeshService* meshSvc = nullptr;
  MessageStore* msgStore = nullptr;

  // State
  ScreenId current = ScreenId::KidHome;
  bool unlockError = false;
  String activePeer;

  // Button-driven UI state
  uint16_t cursor = 0;   // selected item index
  uint16_t page = 0;     // page index for paged views (chat)
  uint16_t listTop = 0;  // top index of visible window in list views

  void go(ScreenId next);

  // Serial/CLI input
  void handleLine(const String& lineIn);

  // Button UI input
  void handleUiEvent(uint8_t e); // UiInputEvent as uint8_t to keep header light
  void uiNext();
  void uiSelect();

  // Render helpers
  void render();
  void renderInbox();
  void renderChat(const String& peer);

  // Helpers
  void resetUiState() { cursor = 0; page = 0; listTop = 0; }
};
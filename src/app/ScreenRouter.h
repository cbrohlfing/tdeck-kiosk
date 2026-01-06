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
  LockManager* lockMgr = nullptr;
  Display* ui = nullptr;
  Input* in = nullptr;
  UiInput* uiIn = nullptr;
  MeshService* meshSvc = nullptr;
  MessageStore* msgStore = nullptr;

  ScreenId current = ScreenId::KidHome;
  bool unlockError = false;
  String activePeer;

  uint16_t cursor = 0;
  uint16_t page = 0;
  uint16_t listTop = 0;

  void go(ScreenId next);

  void handleLine(const String& lineIn);

  void handleUiEvent(uint8_t e);

  void uiNext();
  void uiPrev();
  void uiSelect();
  void uiBack();
  void uiHome();

  void render();
  void renderInbox();
  void renderChat(const String& peer);

  void resetUiState() { cursor = 0; page = 0; listTop = 0; }
};
#pragma once
#include <Arduino.h>
#include "Screens.h"

class LockManager;
class Display;
class Input;
class MeshService;
class MessageStore;

class ScreenRouter {
public:
  void begin(LockManager* lock, Display* display, Input* input, MeshService* mesh, MessageStore* store);
  void tick();

private:
  LockManager* lockMgr = nullptr;
  Display* ui = nullptr;
  Input* in = nullptr;
  MeshService* meshSvc = nullptr;
  MessageStore* msgStore = nullptr;

  ScreenId current = ScreenId::KidHome;
  bool unlockError = false;

  // Chat context (used on Chat screen)
  String activePeer;

  void go(ScreenId next);
  void render();
  void handleLine(const String& line);

  // Helpers
  void renderInbox();
  void renderChat(const String& peer);
};
#include "UiApp.h"
#include "ScreenRouter.h"
#include "../lock/LockManager.h"
#include "../hw/SerialDisplay.h"
#include "../hw/SerialInput.h"
#include "../mesh/FakeMeshService.h"
#include "../model/MessageStore.h"
#include "../model/Message.h"

static LockManager lockMgr;
static ScreenRouter router;
static SerialDisplay display;
static SerialInput input;

static MessageStore store;
static FakeMeshService mesh;

void UiApp::begin() {
  lockMgr.begin();

  mesh.onInbound([](const String& peer, const String& text) {
    Message m;
    m.peer = peer;
    m.text = text;
    m.inbound = true;
    m.tsMs = millis();
    store.add(m);
  });

  mesh.begin();

  router.begin(&lockMgr, &display, &input, &mesh, &store);
}

void UiApp::loop() {
  mesh.tick();
  router.tick();
}
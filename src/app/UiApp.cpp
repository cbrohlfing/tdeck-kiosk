#include "UiApp.h"
#include "ScreenRouter.h"
#include "../lock/LockManager.h"
#include <Arduino.h>

static LockManager lockMgr;
static ScreenRouter router;

void UiApp::begin() {
  lockMgr.begin();
  router.begin(&lockMgr);
}

void UiApp::loop() {
  router.tick();
}
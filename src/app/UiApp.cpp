#include "UiApp.h"
#include "ScreenRouter.h"
#include "../lock/LockManager.h"
#include "../hw/SerialDisplay.h"
#include "../hw/SerialInput.h"

static LockManager lockMgr;
static ScreenRouter router;
static SerialDisplay display;
static SerialInput input;

void UiApp::begin() {
  lockMgr.begin();
  router.begin(&lockMgr, &display, &input);
}

void UiApp::loop() {
  router.tick();
}
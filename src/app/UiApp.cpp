#include "UiApp.h"

#include "../app/ScreenRouter.h"
#include "../hw/Display.h"
#include "../hw/Input.h"
#include "../hw/BoardFactory.h"
#include "../hw/BoardServices.h"

#include "../lock/LockManager.h"
#include "../model/MessageStore.h"

#include "../mesh/MeshService.h"
#include "../mesh/FakeMeshService.h"
#include "../mesh/MeshCoreMeshService.h"

// Only needed so we can call ->tick() on these types
#if defined(HW_HELTEC_V3)
  #include "../hw/BatteryMonitor.h"
  #include "../hw/PowerButtonHeltecV3.h"
#endif

// App state
static LockManager lockMgr;
static MessageStore store;
static ScreenRouter router;

// Mesh backends (still using FakeMesh by default)
static FakeMeshService fakeMesh;
static MeshCoreMeshService meshCoreStub;
static MeshService* mesh = &fakeMesh;

// Board services (display/input/battery/power etc.)
static BoardServices hw;

void UiApp::begin() {
  mesh->begin();

  // Initialize board-specific services (display/input/etc)
  hw = BoardFactory::begin();

  // Router uses display + input
  router.begin(&lockMgr, hw.display, hw.input, mesh, &store);

  if (hw.display) hw.display->line("[boot] UiApp ready");
}

void UiApp::loop() {
#if defined(HW_HELTEC_V3)
  if (hw.battery) hw.battery->tick();
  if (hw.powerButton) hw.powerButton->tick();
#endif

  mesh->tick();
  router.tick();
}
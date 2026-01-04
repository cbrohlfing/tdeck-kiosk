#include "UiApp.h"

#include "../app/ScreenRouter.h"
#include "../hw/SerialDisplay.h"
#include "../hw/SerialInput.h"
#include "../lock/LockManager.h"
#include "../model/MessageStore.h"

#include "../mesh/MeshService.h"
#include "../mesh/FakeMeshService.h"
#include "../mesh/MeshCoreMeshService.h"

#include "../hw/MultiDisplay.h"

#if defined(HW_HELTEC_V3)
  #include "../hw/OledDisplayHeltecV3.h"
  #include "../hw/BatteryMonitor.h"
  #include "../hw/PowerButtonHeltecV3.h"
#endif

// Concrete hardware implementations (Serial-based for now)
static SerialDisplay serialDisplay;
static SerialInput input;

// Optional Heltec OLED + battery
#if defined(HW_HELTEC_V3)
  static BatteryMonitor battery;
  static OledDisplayHeltecV3 oled;
  static MultiDisplay display(&serialDisplay, &oled);

  static PowerButtonHeltecV3 powerBtn;
#else
  // Default: serial-only display
  static Display& display = serialDisplay;
#endif

// App state
static LockManager lockMgr;
static MessageStore store;
static ScreenRouter router;

// Mesh backends
static FakeMeshService fakeMesh;
static MeshCoreMeshService meshCoreStub;

// Select which backend to use.
static MeshService* mesh = &fakeMesh;

void UiApp::begin() {
  mesh->begin();

#if defined(HW_HELTEC_V3)
  battery.begin();
  oled.begin(&battery);

  // Long-press BOOT button to deep sleep
  powerBtn.begin(&display);
#endif

  router.begin(&lockMgr, &display, &input, mesh, &store);

  display.line("[boot] UiApp ready");
}

void UiApp::loop() {
#if defined(HW_HELTEC_V3)
  battery.tick();
  powerBtn.tick();
#endif

  mesh->tick();
  router.tick();
}
#include "UiApp.h"

#include "../app/ScreenRouter.h"
#include "../hw/SerialDisplay.h"
#include "../hw/SerialInput.h"
#include "../lock/LockManager.h"
#include "../model/MessageStore.h"

#include "../mesh/MeshService.h"
#include "../mesh/FakeMeshService.h"
#include "../mesh/MeshCoreMeshService.h"
#include "../mesh/MeshMuxService.h"

// Concrete hardware implementations (Serial-based for now)
static SerialDisplay display;
static SerialInput input;

// App state
static LockManager lockMgr;
static MessageStore store;
static ScreenRouter router;

// Mesh backends
static FakeMeshService fakeMesh;
static MeshCoreMeshService meshCoreStub;

// Mux: chat uses FakeMesh; mc CLI uses MeshCore stub
static MeshMuxService mux(&fakeMesh, &meshCoreStub);

// Single mesh pointer used by the app/router
static MeshService* mesh = &mux;

void UiApp::begin() {
  mesh->begin();
  router.begin(&lockMgr, &display, &input, mesh, &store);
  display.line("[boot] UiApp ready");
}

void UiApp::loop() {
  mesh->tick();
  router.tick();
}
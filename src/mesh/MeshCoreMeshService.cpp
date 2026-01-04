#include "MeshCoreMeshService.h"

void MeshCoreMeshService::begin() {
  // Stub: nothing to initialize yet
}

void MeshCoreMeshService::tick() {
  // Stub: no background work yet
}

bool MeshCoreMeshService::send(const String& peer, const String& text) {
  (void)peer;
  (void)text;
  // Stub: pretend send failed
  return false;
}
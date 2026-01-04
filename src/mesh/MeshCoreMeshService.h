#pragma once

#include "MeshService.h"

// Stub MeshCore backend.
// This compiles, identifies itself, and does nothing else.
class MeshCoreMeshService : public MeshService {
public:
  void begin() override;
  void tick() override;
  bool send(const String& peer, const String& text) override;
};
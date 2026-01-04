#pragma once
#include "MeshService.h"

// Stub backend: does NOT implement real MeshCore yet.
// It only supports backendCli() so "mc help" can work end-to-end.
class MeshCoreMeshService : public MeshService {
public:
  void begin() override;
  void tick() override;

  bool send(const String& peer, const String& text) override;
  void injectInbound(const String& peer, const String& text) override;

  bool backendCli(const String& line, String* out) override;

private:
  bool _ready = false;
};
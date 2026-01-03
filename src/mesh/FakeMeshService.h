#pragma once
#include "MeshService.h"

class FakeMeshService : public MeshService {
public:
  void begin() override {}
  void tick() override {}

  bool send(const String& peer, const String& text) override {
    // Fake send always succeeds
    (void)peer; (void)text;
    return true;
  }

  void injectInbound(const String& peer, const String& text) override {
    if (inboundCb) inboundCb(peer, text);
  }
};
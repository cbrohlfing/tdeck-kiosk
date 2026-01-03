#pragma once
#include <Arduino.h>
#include <functional>

class MeshService {
public:
  virtual ~MeshService() = default;

  // Called once at startup
  virtual void begin() = 0;

  // Called frequently from loop
  virtual void tick() = 0;

  // Send a message to a peer
  virtual bool send(const String& peer, const String& text) = 0;

  // Simulate / deliver an inbound message (used by fake impl)
  virtual void injectInbound(const String& peer, const String& text) = 0;

  // Inbound callback
  void onInbound(std::function<void(const String&, const String&)> cb) { inboundCb = cb; }

protected:
  std::function<void(const String&, const String&)> inboundCb;
};
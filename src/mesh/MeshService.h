#pragma once
#include <Arduino.h>
#include <functional>

class MeshService {
public:
  using InboundCallback = std::function<void(const String& peer, const String& text)>;

  virtual ~MeshService() = default;

  virtual void begin() = 0;
  virtual void tick() = 0;

  virtual bool send(const String& peer, const String& text) = 0;
  virtual void injectInbound(const String& peer, const String& text) = 0;

  void onInbound(InboundCallback cb) { inboundCb = cb; }

  // Optional: backend CLI passthrough. Default = not supported.
  // Return true if handled and write response to out (if provided).
  virtual bool backendCli(const String& line, String* out) {
    (void)line;
    if (out) *out = "";
    return false;
  }

protected:
  InboundCallback inboundCb;
};
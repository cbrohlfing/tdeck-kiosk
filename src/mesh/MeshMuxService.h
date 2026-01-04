#pragma once

#include "MeshService.h"

// Simple mux:
// - send/inbound/tick go to _chat
// - backendCli goes to _cli
class MeshMuxService : public MeshService {
public:
  MeshMuxService(MeshService* chat, MeshService* cli)
  : _chat(chat), _cli(cli) {}

  void begin() override {
    if (_chat) _chat->begin();
    if (_cli)  _cli->begin();
  }

  void tick() override {
    if (_chat) _chat->tick();
    if (_cli)  _cli->tick();
  }

  bool send(const String& peer, const String& text) override {
    return _chat ? _chat->send(peer, text) : false;
  }

  void injectInbound(const String& peer, const String& text) override {
    if (_chat) _chat->injectInbound(peer, text);
  }

  bool backendCli(const String& line, String* out) override {
    return _cli ? _cli->backendCli(line, out) : false;
  }

private:
  MeshService* _chat = nullptr;
  MeshService* _cli  = nullptr;
};
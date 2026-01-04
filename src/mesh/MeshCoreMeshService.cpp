#include "MeshCoreMeshService.h"

void MeshCoreMeshService::begin() {
  _ready = true;
}

void MeshCoreMeshService::tick() {
  // no-op for stub
}

bool MeshCoreMeshService::send(const String& peer, const String& text) {
  (void)peer;
  (void)text;
  // Stub: not wired to radio/mesh yet
  return false;
}

void MeshCoreMeshService::injectInbound(const String& peer, const String& text) {
  if (inboundCb) inboundCb(peer, text);
}

static String trimCopy(const String& s) {
  String t = s;
  t.trim();
  return t;
}

bool MeshCoreMeshService::backendCli(const String& line, String* out) {
  if (!_ready) {
    if (out) *out = "MeshCore stub backend not ready.\n";
    return true;
  }

  String cmd = trimCopy(line);

  if (cmd.length() == 0 || cmd == "help") {
    if (out) {
      *out =
        "MeshCore CLI (STUB)\n"
        "-------------------\n"
        "help                Show this help\n"
        "status              Show stub status\n"
        "note                This is NOT real MeshCore yet\n";
    }
    return true;
  }

  if (cmd == "status") {
    if (out) {
      *out =
        "MeshCore CLI status:\n"
        "  backend: stub\n"
        "  mesh:    not wired\n";
    }
    return true;
  }

  if (out) {
    *out =
      "Unknown mc command: " + cmd + "\n"
      "Try: mc help\n";
  }
  return true;
}
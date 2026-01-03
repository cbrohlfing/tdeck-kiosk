#include "MessageStore.h"

void MessageStore::add(const Message& m) {
  messages.push_back(m);
}

const std::vector<Message>& MessageStore::all() const {
  return messages;
}

std::vector<String> MessageStore::peersMostRecentFirst() const {
  std::vector<String> peers;
  // Walk from back to front so newest first
  for (int i = (int)messages.size() - 1; i >= 0; --i) {
    const String& p = messages[i].peer;
    bool seen = false;
    for (auto& existing : peers) {
      if (existing == p) { seen = true; break; }
    }
    if (!seen) peers.push_back(p);
  }
  return peers;
}

std::vector<Message> MessageStore::messagesFor(const String& peer) const {
  std::vector<Message> out;
  out.reserve(messages.size());
  for (auto& m : messages) {
    if (m.peer == peer) out.push_back(m);
  }
  return out;
}
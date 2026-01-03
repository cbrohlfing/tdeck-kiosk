#pragma once
#include <Arduino.h>
#include <vector>
#include "Message.h"

class MessageStore {
public:
  void add(const Message& m);
  const std::vector<Message>& all() const;

  // Returns unique peers ordered by most-recent activity
  std::vector<String> peersMostRecentFirst() const;

  // Returns messages for a peer (in chronological order)
  std::vector<Message> messagesFor(const String& peer) const;

private:
  std::vector<Message> messages;
};
// /src/hw/UiInput.h
#pragma once
#include <stdint.h>
#include "UiInputEvent.h"

// Very small event queue for UI navigation.
// Board code "posts" events, app code "takes" them.
class UiInput {
public:
  void post(UiInputEvent e) {
    if (e == UiInputEvent::None) return;

    // If full, drop the oldest (advance tail)
    if (full_) {
      tail_ = (uint8_t)((tail_ + 1) % kCap);
      full_ = false;
    }

    q_[head_] = e;
    head_ = (uint8_t)((head_ + 1) % kCap);
    if (head_ == tail_) full_ = true;
  }

  bool take(UiInputEvent& out) {
    if (empty()) return false;
    out = q_[tail_];
    tail_ = (uint8_t)((tail_ + 1) % kCap);
    full_ = false;
    return true;
  }

  bool empty() const { return (!full_ && head_ == tail_); }

private:
  static constexpr uint8_t kCap = 8;
  UiInputEvent q_[kCap] = {};
  uint8_t head_ = 0;
  uint8_t tail_ = 0;
  bool full_ = false;
};
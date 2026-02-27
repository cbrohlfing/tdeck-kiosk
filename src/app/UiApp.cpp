#include "UiApp.h"

void UiApp::begin(Display* display, UiInput* input) {
  display_ = display;
  input_   = input;

  lastEvent_   = UiInputEvent::None;
  lastKeycode_ = 0x00;
  eventCount_  = 0;

  dirty_ = true;
  lastDrawMs_ = 0;

  draw_();
}

const char* UiApp::eventName_(UiInputEvent e) {
  switch (e) {
    case UiInputEvent::None:    return "None";

    // Use ONLY the primary names (NavNext/NavPrev) to avoid duplicate case labels
    case UiInputEvent::NavNext: return "NavNext";
    case UiInputEvent::NavPrev: return "NavPrev";

    case UiInputEvent::Select:  return "Select";
    case UiInputEvent::Back:    return "Back";
    case UiInputEvent::Home:    return "Home";
    default:                    return "?";
  }
}

void UiApp::loop() {
  UiInputEvent e;
  bool gotAny = false;

  while (input_ && input_->take(e)) {
    gotAny = true;
    lastEvent_ = e;
    eventCount_++;

    // Optional: fake keycode display so you can see activity clearly
    switch (e) {
      case UiInputEvent::NavNext: lastKeycode_ = 0x0F; break;
      case UiInputEvent::NavPrev: lastKeycode_ = 0x1F; break;
      case UiInputEvent::Select:  lastKeycode_ = 0x76; break;
      case UiInputEvent::Back:    lastKeycode_ = 0xD8; break;
      case UiInputEvent::Home:    lastKeycode_ = 0x5A; break; // arbitrary
      default: break;
    }
  }

  if (gotAny) dirty_ = true;

  // Heartbeat redraw (1 Hz). Comment out for ONLY-on-event redraw.
  uint32_t now = millis();
  //if (!dirty_ && (now - lastDrawMs_) >= 1000) {
  //  dirty_ = true;
  //}

  if (dirty_) {
    draw_();
    dirty_ = false;
    lastDrawMs_ = millis();
  }
}

void UiApp::draw_() {
  if (!display_) return;

  display_->clear();
  display_->line("Keyboard Debug");
  display_->line("----------------");
  display_->line("Event:");
  display_->line(String(eventName_(lastEvent_)));
  display_->line("");

  display_->line("Keycode:");
  char buf[8];
  snprintf(buf, sizeof(buf), "0x%02X", (unsigned)lastKeycode_);
  display_->line(String(buf));
  display_->line("");

  display_->line("Count:");
  display_->line(String((unsigned long)eventCount_));
}
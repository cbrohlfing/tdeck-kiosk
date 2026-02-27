#include <Arduino.h>

#include "app/UiApp.h"
#include "hw/BoardFactory.h"
#include "hw/BoardServices.h"

static UiApp gApp;
static BoardServices gHw;

void setup() {
  Serial.begin(115200);
  delay(200);

  Serial.println("BOOT: hello from T-Deck");

  // Build board services (Display + UiInput + Trackball + Keyboard, etc.)
  gHw = BoardFactory::begin();

  // UI app consumes Display + UiInput
  gApp.begin(gHw.display, gHw.uiInput);
}

void loop() {
  // Let board devices post UiInputEvents (keyboard/trackball/etc)
  BoardFactory::tick(gHw);

  // Let the UI consume events + render
  gApp.loop();

  delay(1);
}
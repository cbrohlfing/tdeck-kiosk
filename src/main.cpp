#include <Arduino.h>
#include "app/UiApp.h"

UiApp app;

void setup() {
  Serial.begin(115200);
  delay(2000);                 // <-- change: give USB serial time
  Serial.println("BOOT: hello from T-Deck");
  app.begin();
}

void loop() {
  app.loop();
}
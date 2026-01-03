#include <Arduino.h>
#include "app/UiApp.h"

UiApp app;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("T-Deck Kid Mode firmware booting...");
  app.begin();
}

void loop() {
  app.loop();
}
#include <Arduino.h>

#include "app/UiApp.h"
#include "hw/BoardFactory.h"
#include "hw/BoardServices.h"

// ============================================================
// Toggle at build time:
//   -DDISPLAY_SMOKETEST=1   -> runs TFT smoketest
//   (unset)                 -> runs normal app
// ============================================================

#if defined(DISPLAY_SMOKETEST)
  #include <SPI.h>
  #include <TFT_eSPI.h>

  // T-Deck Plus / T-Deck pins (from LilyGO examples)
  static constexpr int PWR_ON_PIN  = 10;
  static constexpr int TFT_CS_PIN  = 12;
  static constexpr int TFT_DC_PIN  = 11;
  static constexpr int TFT_BL_PIN  = 42;

  static constexpr int SPI_SCK_PIN  = 40;
  static constexpr int SPI_MISO_PIN = 38;
  static constexpr int SPI_MOSI_PIN = 41;

  TFT_eSPI tft;

  static void backlightOn() {
    pinMode(TFT_BL_PIN, OUTPUT);
    digitalWrite(TFT_BL_PIN, HIGH); // if dim/off on your unit, we can adjust later
  }

  static void peripheralsOn() {
    pinMode(PWR_ON_PIN, OUTPUT);
    digitalWrite(PWR_ON_PIN, HIGH);
    delay(20);
  }

  static void deselectSpiDevices() {
    // Good practice: ensure CS pins are high before starting SPI
    pinMode(TFT_CS_PIN, OUTPUT);
    digitalWrite(TFT_CS_PIN, HIGH);
  }

  void setup() {
    Serial.begin(115200);
    delay(200);
    Serial.println("BOOT: display smoketest");

    peripheralsOn();
    deselectSpiDevices();
    backlightOn();

    // Start SPI bus (matches LilyGO UnitTest style)
    SPI.begin(SPI_SCK_PIN, SPI_MISO_PIN, SPI_MOSI_PIN, -1);
    delay(10);

    // LilyGO fork typically uses begin(); upstream often uses init()
    tft.begin();
    tft.setRotation(1);

    tft.fillScreen(TFT_RED);   delay(250);
    tft.fillScreen(TFT_GREEN); delay(250);
    tft.fillScreen(TFT_BLUE);  delay(250);

    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("T-Deck Plus");
    tft.println("TFT OK?");
  }

  void loop() {
    delay(1000);
  }

#else
  // ----------------------------
  // Normal app path (unchanged)
  // ----------------------------
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
#endif
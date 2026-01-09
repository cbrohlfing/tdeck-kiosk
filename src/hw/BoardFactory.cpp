// /src/hw/BoardFactory.cpp
#include "BoardFactory.h"

#include "BoardServices.h"

#include "Display.h"
#include "Input.h"
#include "SerialDisplay.h"
#include "SerialInput.h"
#include "UiInput.h"

#if defined(HW_HELTEC_V3)
  #include "../boards/heltec_v3/BatteryMonitor.h"
  #include "../boards/heltec_v3/OledDisplayHeltecV3.h"
  #include "../boards/heltec_v3/PowerButtonHeltecV3.h"
  #include "MultiDisplay.h"
#endif

#if defined(HW_TDECK) || defined(HW_TDECK_PLUS)
  #include "../boards/tdeck/TDeckTrackball.h"
#endif

#if defined(HW_TDECK_PLUS)
  #include "../boards/tdeck/TDeckKeyboard.h"
#endif

// --- Shared concrete instances (static lifetime) ---
static SerialDisplay gSerialDisplay;
static SerialInput gSerialInput;
static UiInput gUiInput;

#if defined(HW_HELTEC_V3)
static BatteryMonitor gBattery;
static OledDisplayHeltecV3 gOled;
static PowerButtonHeltecV3 gPowerButton;
static MultiDisplay gDisplay(&gSerialDisplay, &gOled);
#else
static Display& gDisplay = gSerialDisplay;
#endif

#if defined(HW_TDECK) || defined(HW_TDECK_PLUS)
static TDeckTrackball gTrackball;
#endif

#if defined(HW_TDECK_PLUS)
static TDeckKeyboard gKeyboard;
#endif

BoardServices BoardFactory::begin() {
  BoardServices hw;
  hw.display = &gDisplay;
  hw.input = &gSerialInput;
  hw.uiInput = &gUiInput;

#if defined(HW_HELTEC_V3)
  gBattery.begin();
  gOled.begin(&gBattery);
  gPowerButton.begin(hw.display, hw.uiInput);

  hw.battery = &gBattery;
  hw.powerButton = &gPowerButton;

  if (hw.display) hw.display->line("[boot] BoardFactory: Heltec V3 init ok");

#elif defined(HW_TDECK)
  gTrackball.begin(hw.display, hw.uiInput);
  hw.trackball = &gTrackball;

  if (hw.display) hw.display->line("[boot] BoardFactory: T-Deck init ok");

#elif defined(HW_TDECK_PLUS)
  gTrackball.begin(hw.display, hw.uiInput);
  gKeyboard.begin(hw.display, hw.uiInput);

  hw.trackball = &gTrackball;
  hw.keyboard = &gKeyboard;

  if (hw.display) hw.display->line("[boot] BoardFactory: T-Deck Plus init ok");

#else
  if (hw.display) hw.display->line("[boot] BoardFactory: Serial-only init ok");
#endif

  return hw;
}

void BoardFactory::tick(BoardServices& hw) {
#if defined(HW_HELTEC_V3)
  if (hw.battery) hw.battery->tick();
  if (hw.powerButton) hw.powerButton->tick();

#elif defined(HW_TDECK) || defined(HW_TDECK_PLUS)
  if (hw.trackball) hw.trackball->tick();

  #if defined(HW_TDECK_PLUS)
  if (hw.keyboard) hw.keyboard->tick();
  #endif

#else
  (void)hw;
#endif
}
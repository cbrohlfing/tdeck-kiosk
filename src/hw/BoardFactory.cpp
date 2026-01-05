#include "BoardFactory.h"

#include "BoardServices.h"

#include "Display.h"
#include "Input.h"
#include "SerialDisplay.h"
#include "SerialInput.h"

#if defined(HW_HELTEC_V3)
  #include "MultiDisplay.h"
  #include "OledDisplayHeltecV3.h"
  #include "BatteryMonitor.h"
  #include "PowerButtonHeltecV3.h"
#endif

// ----- Concrete singletons for this firmware build -----
static SerialDisplay gSerialDisplay;
static SerialInput gSerialInput;

#if defined(HW_HELTEC_V3)
  static BatteryMonitor gBattery;
  static OledDisplayHeltecV3 gOled;
  static MultiDisplay gDisplay(&gSerialDisplay, &gOled);
  static PowerButtonHeltecV3 gPowerButton;
#else
  // Non-Heltec builds: serial-only for now
  static Display& gDisplay = gSerialDisplay;
#endif

BoardServices BoardFactory::begin() {
  BoardServices s;

#if defined(HW_HELTEC_V3)
  gBattery.begin();
  gOled.begin(&gBattery);

  s.display = &gDisplay;
  s.input = &gSerialInput;
  s.battery = &gBattery;

  // NOTE: We pass the same display pointer so power logs go to both serial + OLED.
  gPowerButton.begin(s.display);
  s.powerButton = &gPowerButton;

  if (s.display) s.display->line("[boot] BoardFactory: Heltec V3 init ok");
#else
  s.display = &gSerialDisplay;
  s.input = &gSerialInput;

  if (s.display) s.display->line("[boot] BoardFactory: Serial-only init ok");
#endif

  return s;
}
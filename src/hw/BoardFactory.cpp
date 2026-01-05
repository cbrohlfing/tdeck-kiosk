#include "BoardFactory.h"

#include "Display.h"
#include "Input.h"
#include "SerialDisplay.h"
#include "SerialInput.h"

#if defined(HW_HELTEC_V3)
  #include "BatteryMonitor.h"
  #include "OledDisplayHeltecV3.h"
  #include "PowerButtonHeltecV3.h"
  #include "MultiDisplay.h"
#endif

// --- Shared concrete instances (static lifetime) ---
static SerialDisplay gSerialDisplay;
static SerialInput gSerialInput;

#if defined(HW_HELTEC_V3)
static BatteryMonitor gBattery;
static OledDisplayHeltecV3 gOled;
static PowerButtonHeltecV3 gPowerButton;
static MultiDisplay gDisplay(&gSerialDisplay, &gOled);
#else
static Display& gDisplay = gSerialDisplay;
#endif

BoardServices BoardFactory::begin() {
  BoardServices hw;
  hw.display = &gDisplay;
  hw.input = &gSerialInput;

#if defined(HW_HELTEC_V3)
  gBattery.begin();
  gOled.begin(&gBattery);
  gPowerButton.begin(hw.display);

  hw.battery = &gBattery;
  hw.powerButton = &gPowerButton;

  if (hw.display) hw.display->line("[boot] BoardFactory: Heltec V3 init ok");
#else
  if (hw.display) hw.display->line("[boot] BoardFactory: Serial-only init ok");
#endif

  return hw;
}

void BoardFactory::tick(BoardServices& hw) {
#if defined(HW_HELTEC_V3)
  if (hw.battery) hw.battery->tick();
  if (hw.powerButton) hw.powerButton->tick();
#else
  (void)hw;
#endif
}
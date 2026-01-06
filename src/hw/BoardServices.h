#pragma once

class Display;
class Input;
class UiInput;

#if defined(HW_HELTEC_V3)
class BatteryMonitor;
class PowerButtonHeltecV3;
#endif

struct BoardServices {
  Display* display = nullptr;
  Input* input = nullptr;
  UiInput* uiInput = nullptr;

#if defined(HW_HELTEC_V3)
  BatteryMonitor* battery = nullptr;
  PowerButtonHeltecV3* powerButton = nullptr;
#endif
};
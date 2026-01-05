#pragma once

// Exactly ONE of these should be true based on build flags.
// We keep the existing HW_HELTEC_V3 define you already use.

#if defined(HW_HELTEC_V3)
  #define BOARD_HELTEC_V3 1
#else
  #define BOARD_GENERIC_SERIAL 1
#endif
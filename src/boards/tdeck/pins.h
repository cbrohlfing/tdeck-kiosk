// /src/boards/tdeck/pins.h
#pragma once

// T-Deck trackball GPIOs.
// Values from LilyGO T-Deck wiki pin overview:
//   G01=3, G02=2, G03=15, G04=1, BOOT=0
// Typical direction mapping used in the wild:
//   Up=G01, Right=G02, Down=G03, Left=G04, Press=BOOT
#if defined(HW_TDECK) || defined(HW_TDECK_PLUS)
static constexpr int TDECK_TRACKBALL_UP_PIN    = 3;
static constexpr int TDECK_TRACKBALL_RIGHT_PIN = 2;
static constexpr int TDECK_TRACKBALL_DOWN_PIN  = 15;
static constexpr int TDECK_TRACKBALL_LEFT_PIN  = 1;
static constexpr int TDECK_TRACKBALL_PRESS_PIN = 0;

static constexpr int TDECK_POWERON_PIN   = 10;
static constexpr int TDECK_TFT_CS_PIN    = 12;
static constexpr int TDECK_TFT_DC_PIN    = 11;
static constexpr int TDECK_TFT_BL_PIN    = 42;
static constexpr int TDECK_SPI_MOSI_PIN  = 41;
static constexpr int TDECK_SPI_MISO_PIN  = 38;
static constexpr int TDECK_SPI_SCK_PIN   = 40;
#else
static constexpr int TDECK_TRACKBALL_UP_PIN    = -1;
static constexpr int TDECK_TRACKBALL_RIGHT_PIN = -1;
static constexpr int TDECK_TRACKBALL_DOWN_PIN  = -1;
static constexpr int TDECK_TRACKBALL_LEFT_PIN  = -1;
static constexpr int TDECK_TRACKBALL_PRESS_PIN = -1;

static constexpr int TDECK_POWERON_PIN   = -1;
static constexpr int TDECK_TFT_CS_PIN    = -1;
static constexpr int TDECK_TFT_DC_PIN    = -1;
static constexpr int TDECK_TFT_BL_PIN    = -1;
static constexpr int TDECK_SPI_MOSI_PIN  = -1;
static constexpr int TDECK_SPI_MISO_PIN  = -1;
static constexpr int TDECK_SPI_SCK_PIN   = -1;
#endif
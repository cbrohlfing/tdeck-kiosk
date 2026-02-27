#pragma once

// Mark setup as provided by the project
#define USER_SETUP_LOADED 1

// ---- Driver ----
#define ST7789_DRIVER

// ---- Resolution ----
// T-Deck is 320x240 (landscape). TFT_eSPI uses rotation to map this.
// Keep these as the native panel dims:
#define TFT_WIDTH  320
#define TFT_HEIGHT 240

// ---- Pins ----
#define TFT_MOSI 41
#define TFT_MISO 38
#define TFT_SCLK 40

#define TFT_CS   12
#define TFT_DC   11
#define TFT_RST  -1     // T-Deck typically doesn’t use a dedicated TFT reset pin

// ---- Backlight (we'll control it in code too) ----
#define TFT_BL   42

// ---- SPI speed ----
#define SPI_FREQUENCY  40000000

// Optional: fonts (keep minimal for now)
// #define LOAD_GLCD
// #define LOAD_FONT2
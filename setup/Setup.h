
#pragma once

// Flags for TFT_MADCTL
#define TFT_MAD_MY  0x80
#define TFT_MAD_MX  0x40
#define TFT_MAD_YX  0x20
#define TFT_MAD_MV  0x10
#define TFT_MAD_RGB 0x00
#define TFT_MAD_BGR 0x08
#define TFT_MAD_MH  0x04
#define TFT_MAD_SS  0x02
#define TFT_MAD_GS  0x01

// ------------------------------- Display ------------------------------------

#define ROTATION_VTB 2    // vertical top to bottom
#define ROTATION_VBT 0    // vertical bottom to top
#define ROTATION_HLR 1    // horizontal left to right
#define ROTATION_HRL 3    // horizontal right to left

#define DEFAULT_LED_PIN   PICO_DEFAULT_LED_PIN


#define TFT_REV 0

#define TFT_WIDTH 240
#define TFT_HEIGHT 320

// ---------------------------- Touch Screen ----------------------------------

// select one of or none
  #define TOUCH_SPI
  #define TOUCH_SPI_SPEED         2 * 1000 * 1000          //  2 MHz

// don't comment it if T.CS is connected to this pin
  #define TOUCH_SPI_CS   22     // Chip select pin (T_CS) of touch screen

  #define TOUCH_REV 0
  #define TOUCH_ROTATION touch_7789

// TouchScreen edges of the display in range of 0..4095
  #define TS_LEFT 200
  #define TS_TOP 350
  #define TS_RIGHT 200
  #define TS_BOTTOM 150

// -------------------------------I2C params ----------------------------------

  #define I2C0_SDA_PIN 4
  #define I2C0_SCL_PIN 5

  #define I2C1_SDA_PIN 6
  #define I2C1_SCL_PIN 7

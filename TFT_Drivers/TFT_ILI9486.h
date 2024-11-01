
#pragma once

// Change the width and height if required (defined in portrait mode)
// or use the constructor to over-ride defaults
#define TFT_WIDTH  320
#define TFT_HEIGHT 480

// For Raspberry Pi ILI9486 only with a modified board to add a write strobe:
#if defined (TFT_WR) && defined (RPI_ILI9486_DRIVER)
  #define RPI_WRITE_STROBE
#endif

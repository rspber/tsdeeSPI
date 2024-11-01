
#pragma once

// Change the width and height if required (defined in portrait mode)
// or use the constructor to over-ride defaults
#ifndef TFT_WIDTH
  #define TFT_WIDTH  128
#endif
#ifndef TFT_HEIGHT
  #define TFT_HEIGHT 160
#endif

#if (TFT_HEIGHT == 128) && (TFT_WIDTH == 128)
  #define CGRAM_OFFSET
#endif


#pragma once


#define COLOR_565

#if defined(COLOR_565)
  #define TFT_PIXFMT_VALUE 0x55
#else
  #define TFT_PIXFMT_VALUE 0x66
#endif


// ------------------- Constants to use in examples --------------------------

#define ROTATION_VTB 2    // vertical top to bottom
#define ROTATION_VBT 0    // vertical bottom to top
#define ROTATION_HLR 1    // horizontal left to right
#define ROTATION_HRL 3    // horizontal right to left

#define DEFAULT_LED_PIN PICO_DEFAULT_LED_PIN

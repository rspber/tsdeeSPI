
#pragma once


#define COLOR_565

#if defined(COLOR_565)
  #define TFT_PIXFMT_VALUE 0x55
#else
  #define TFT_PIXFMT_VALUE 0x66
#endif

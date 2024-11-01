
#pragma once

// Common TFT commands

#define TFT_NOP     0x00
#define TFT_SWRST   0x01
#define TFT_SWRESET   0x01

#define TFT_SLPIN   0x10
#define TFT_SLPOUT  0x11
#define TFT_NORON   0x13

#define TFT_INVOFF  0x20
#define TFT_INVON   0x21
#define TFT_DISPOFF 0x28
#define TFT_DISPON  0x29
#define TFT_CASET   0x2A
#define TFT_PASET   0x2B
#define TFT_RAMWR   0x2C
#define TFT_RAMRD   0x2E
#define TFT_MADCTL  0x36
#define TFT_VSCRSADD    0x37    // Vertical scrolling start address
#define TFT_COLMOD  0x3A
#define TFT_PIXFMT   0x3A

#define TFT_IDXRD   0xDD // ILI9341 only, indexed control register read

// Delay between some initialisation commands
#define TFT_INIT_DELAY 0x80 // Not used unless commandlist invoked


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


/*
#ifdef TFT_RGB_ORDER
  #if (TFT_RGB_ORDER == 1)
    #define TFT_MAD_COLOR_ORDER TFT_MAD_RGB
  #else
    #define TFT_MAD_COLOR_ORDER TFT_MAD_BGR
  #endif
#else
  #ifdef CGRAM_OFFSET
    #define TFT_MAD_COLOR_ORDER TFT_MAD_BGR
  #else
    #define TFT_MAD_COLOR_ORDER TFT_MAD_RGB
  #endif
#endif
*/

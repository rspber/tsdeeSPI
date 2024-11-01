
#pragma once

// Common TFT commands

#define TFT_NOP         0x00    // No-op register
#define TFT_SWRST   0x01
#define TFT_SWRESET     0x01    // Software reset register
#define TFT_SLPIN       0x10    // Enter sleep mode
#define TFT_SLPOUT      0x11    // Exit sleep mode
#define TFT_PTLON       0x12    // Partial mode on
#define TFT_NORON       0x13    // Normal display mode on
#define TFT_INVOFF      0x20    // Display inversion off
#define TFT_INVON       0x21    // Display inversion on
#define TFT_DISPOFF     0x28    // Display off
#define TFT_DISPON      0x29    // Display on
#define TFT_CASET       0x2A    // Column address set
#define TFT_PASET       0x2B    // Page address set
#define TFT_RAMWR       0x2C    // Memory write
#define TFT_RAMRD       0x2E    // Memory read
#define TFT_PTLAR       0x30    // Partial area
#define TFT_VSCRDEF     0x33    // Vertical scrolling definition
#define TFT_TEAROFF     0x34    // Tearing effect line off
#define TFT_TEARON      0x35    // Tearing effect line on
#define TFT_MADCTL      0x36    // Memory data access control
#define TFT_VSCRSADD    0x37    // Vertical scrolling start address
#define TFT_IDLEOFF     0x38    // Exit idle mode
#define TFT_IDLEON      0x39    // Enter idle mode
#define TFT_COLMOD  0x3A
#define TFT_PIXFMT      0x3A    // COLMOD: Pixel format set
#define TFT_RWRCNT      0x3C    // Write memory continue
#define TFT_RRDNCT      0x3E    // Read memory continue
#define TFT_TEARLINE    0x44    // Set tear scanline
#define TFT_RDSCLN      0x45    // Get scanline
#define TFT_WRDISBRI    0x51    // Write display brightness
#define TFT_RDDISBRI    0x52    // Read display brightness
#define TFT_WRCTRLDIS   0x53    // Write ctrl display
#define TFT_RDCTRLDIS   0x54    // Read ctrl display
#define TFT_WRITE_CABC  0x55    // Write Content Adaptive Brightness Control
#define TFT_READ_CABC   0x56    // Read Content Adaptive Brightness Control
#define TFT_WRCABCMIN   0x5E    // Write CABC Minimum Brightness
#define TFT_RDCABCMIN   0x5F    // Read CABC Minimum Brightness



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

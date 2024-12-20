/*
  TFT SCREEN

  Copyright (c) 2024, rspber (https://github.com/rspber)

*/

#pragma once

#include <TSD_SCREEN.h>

// common ILIxxxx commands

#define TFT_NOP         0x00    // No-op register
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

#define TFT_IDXRD       0xD9    // undocumented

class TFT_SCREEN : public TSD_SCREEN {
public:
  TFT_SCREEN() : TSD_SCREEN(TFT_WIDTH, TFT_HEIGHT)
  {
  }
//  const char* protocol();
//  void disp_info(char buf[], const int len, const int width, const int height, const char* ext);

//
  rgb_t readPixel(clip_t& clip, int16_t x, int16_t y) override { return 0; }
//  void readRegister(uint8_t* buf, const uint8_t reg, int8_t len);

//  rgb_t innerReadPixel(int16_t x, int16_t y);

#ifdef OVERLAID
  void drawClippedPixel(const int16_t x, const int16_t y, const rgb_t color) override {};
  void drawClippedPixelRec(const int16_t x, const int16_t y, const int16_t w, const int16_t h, const rgb_t color) override {};
#endif

  size_t write(uint8_t) override { return 0; };

// --- the DMA ---------------------------------------------------------------

  bool initDMA(bool ctrl_cs = 0);
  void deInitDMA();
  void startUsingDMA();
  void endUsingDMA();

private:
  void sendMDTBuffer16(const uint8_t* buffer, const int32_t len);
  void sendMDTBuffer24(const uint8_t* buffer, const int32_t len);

  bool dmaBusy();
  void dmaWait();
  void dma_sendMDTBuffer16(const uint8_t* buff, const int32_t len);

  bool dma_enabled;
  bool useDMA;
};

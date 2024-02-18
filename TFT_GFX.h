/*
  Graphics level

  Copyright (c) 2024, rspber (https://github.com/rspber)

  Based on: TFT_eSPI

  Originally notes below:
*/

/***************************************************
  Arduino TFT graphics library targeted at ESP8266
  and ESP32 based boards.

  This is a stand-alone library that contains the
  hardware driver, the graphics functions and the
  proportional fonts.

  The built-in fonts 4, 6, 7 and 8 are Run Length
  Encoded (RLE) to reduce the FLASH footprint.

  Last review/edit by Bodmer: 04/02/22
 ****************************************************/

#pragma once

#define TFT_ESPI_VERSION "2.5.0"

#include "TFT_eeSPI.h"

/***************************************************************************************
**                         Section 1: Load required header files
***************************************************************************************/

/***************************************************************************************
**                         Section 2: Load library and processor specific header files
***************************************************************************************/

/***************************************************************************************
**                         Section 3: Interface setup
***************************************************************************************/

/***************************************************************************************
**                         Section 4: Setup fonts
***************************************************************************************/

/***************************************************************************************
**                         Section 5: Font datum enumeration
***************************************************************************************/

/***************************************************************************************
**                         Section 6: Colour enumeration
***************************************************************************************/

/***************************************************************************************
**                         Section 7: Diagnostic support
***************************************************************************************/

/***************************************************************************************
**                         Section 8: Class member and support functions
***************************************************************************************/

// Class functions and variables
class TFT_GFX : public TFT_eeSPI { friend class TFT_eSprite; // Sprite class has access to protected members

  friend class TFT_CHAR;

 //--------------------------------------- public ------------------------------------//
public:

  TFT_GFX();

  // These are virtual so the TFT_eSprite class can override them with sprite specific functions
  virtual void
                   drawLine(clip_t& clip, int32_t xs, int32_t ys, int32_t xe, int32_t ye, uint32_t color),
                   drawFastVLine(clip_t& clip, int32_t x, int32_t y, int32_t h, uint32_t color),
                   drawFastHLine(clip_t& clip, int32_t x, int32_t y, int32_t w, uint32_t color),
                   fillRect(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color);

  // Graphics drawing
  void
           drawRect(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color),
           drawRoundRect(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, uint32_t color),
           fillRoundRect(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, uint32_t color);

  void     fillRectVGradient(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color1, uint32_t color2);
  void     fillRectHGradient(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color1, uint32_t color2);

  void     drawCircle(clip_t& clip, int32_t x, int32_t y, int32_t r, uint32_t color),
           drawCircleHelper(clip_t& clip, int32_t x, int32_t y, int32_t r, uint8_t cornername, uint32_t color),
           fillCircle(clip_t& clip, int32_t x, int32_t y, int32_t r, uint32_t color),
           fillCircleHelper(clip_t& clip, int32_t x, int32_t y, int32_t r, uint8_t cornername, int32_t delta, uint32_t color),

           drawEllipse(clip_t& clip, int16_t x, int16_t y, int32_t rx, int32_t ry, uint16_t color),
           fillEllipse(clip_t& clip, int16_t x, int16_t y, int32_t rx, int32_t ry, uint16_t color),

           //                 Corner 1               Corner 2               Corner 3
           drawTriangle(clip_t& clip, int32_t x1,int32_t y1, int32_t x2,int32_t y2, int32_t x3,int32_t y3, uint32_t color),
           fillTriangle(clip_t& clip, int32_t x1,int32_t y1, int32_t x2,int32_t y2, int32_t x3,int32_t y3, uint32_t color);


  // Smooth (anti-aliased) graphics drawing
           // Draw a pixel blended with the background pixel colour (bg_color) specified,  return blended colour
           // If the bg_color is not specified, the background pixel colour will be read from TFT or sprite
  uint16_t drawPixel(clip_t& clip, int32_t x, int32_t y, uint32_t color, uint8_t alpha, uint32_t bg_color = 0x00FFFFFF);

  using TFT_eeSPI::drawPixel;

           // Draw an anti-aliased (smooth) arc between start and end angles. Arc ends are anti-aliased.
           // By default the arc is drawn with square ends unless the "roundEnds" parameter is included and set true
           // Angle = 0 is at 6 o'clock position, 90 at 9 o'clock etc. The angles must be in range 0-360 or they will be clipped to these limits
           // The start angle may be larger than the end angle. Arcs are always drawn clockwise from the start angle.
  void     drawSmoothArc(wh_clip_t& clip, int32_t x, int32_t y, int32_t r, int32_t ir, int32_t startAngle, int32_t endAngle, uint32_t fg_color, uint32_t bg_color, bool roundEnds = false);
           // As per "drawSmoothArc" except the ends of the arc are NOT anti-aliased, this facilitates dynamic arc length changes with
           // arc segments and ensures clean segment joints.
           // The sides of the arc are anti-aliased by default. If smoothArc is false sides will NOT be anti-aliased
  void     drawArc(clip_t& clip, int32_t x, int32_t y, int32_t r, int32_t ir, int32_t startAngle, int32_t endAngle, uint32_t fg_color, uint32_t bg_color, bool smoothArc = true);

           // Draw an anti-aliased filled circle at x, y with radius r
           // Note: The thickness of line is 3 pixels to reduce the visible "braiding" effect of anti-aliasing narrow lines
           //       this means the inner anti-alias zone is always at r-1 and the outer zone at r+1
  void     drawSmoothCircle(clip_t& clip, int32_t x, int32_t y, int32_t r, uint32_t fg_color, uint32_t bg_color);

           // Draw an anti-aliased filled circle at x, y with radius r
           // If bg_color is not included the background pixel colour will be read from TFT or sprite
  void     fillSmoothCircle(clip_t& clip, int32_t x, int32_t y, int32_t r, uint32_t color, uint32_t bg_color = 0x00FFFFFF);

           // Draw a rounded rectangle that has a line thickness of r-ir+1 and bounding box defined by x,y and w,h
           // The outer corner radius is r, inner corner radius is ir
           // The inside and outside of the border are anti-aliased
  void     drawSmoothRoundRect(clip_t& clip, int32_t x, int32_t y, int32_t r, int32_t ir, int32_t w, int32_t h, uint32_t fg_color, uint32_t bg_color = 0x00FFFFFF, uint8_t quadrants = 0xF);

           // Draw a filled rounded rectangle , corner radius r and bounding box defined by x,y and w,h
  void     fillSmoothRoundRect(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, uint32_t color, uint32_t bg_color = 0x00FFFFFF);

           // Draw a small anti-aliased filled circle at ax,ay with radius r (uses drawWideLine)
           // If bg_color is not included the background pixel colour will be read from TFT or sprite
  void     drawSpot(clip_t& clip, float ax, float ay, float r, uint32_t fg_color, uint32_t bg_color = 0x00FFFFFF);

           // Draw an anti-aliased wide line from ax,ay to bx,by width wd with radiused ends (radius is wd/2)
           // If bg_color is not included the background pixel colour will be read from TFT or sprite
  void     drawWideLine(clip_t& clip, float ax, float ay, float bx, float by, float wd, uint32_t fg_color, uint32_t bg_color = 0x00FFFFFF);

           // Draw an anti-aliased wide line from ax,ay to bx,by with different width at each end aw, bw and with radiused ends
           // If bg_color is not included the background pixel colour will be read from TFT or sprite
  void     drawWedgeLine(wh_clip_t& clip, float ax, float ay, float bx, float by, float aw, float bw, uint32_t fg_color, uint32_t bg_color = 0x00FFFFFF);


           // Draw bitmap
  void     drawBitmap(clip_t& clip, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t fgcolor),
           drawBitmap(clip_t& clip, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t fgcolor, uint16_t bgcolor),
           drawXBitmap(clip_t& clip, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t fgcolor),
           drawXBitmap(clip_t& clip, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t fgcolor, uint16_t bgcolor);

           // Write a block of pixels to the screen which have been read by readRect()
  void     pushRect(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data);

           // These are used to render images or sprites stored in RAM arrays (used by Sprite class for 16bpp Sprites)
  void     pushImage(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, bool swapBytes, uint16_t *data);
  void     pushImage(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, bool swapBytes, uint16_t *data, uint16_t transparent);

           // These are used to render images stored in FLASH (PROGMEM)
  void     pushImage(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, bool swapBytes, const uint16_t *data, uint16_t transparent);
  void     pushImage(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, bool swapBytes, const uint16_t *data);

           // These are used by Sprite class pushSprite() member function for 1, 4 and 8 bits per pixel (bpp) colours
           // They are not intended to be used with user sketches (but could be)
           // Set bpp8 true for 8bpp sprites, false otherwise. The cmap pointer must be specified for 4bpp
  void     pushImage(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, uint16_t fg, uint16_t bg, uint8_t  *data, bool bpp8 = true, uint16_t *cmap = nullptr);
  void     pushImage(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, uint16_t fg, uint16_t bg, uint8_t  *data, uint8_t  transparent, bool bpp8 = true, uint16_t *cmap = nullptr);
           // FLASH version
  void     pushImage(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, uint16_t fg, uint16_t bg, const uint8_t *data, bool bpp8,  uint16_t *cmap = nullptr);

           // Render a 16 bit colour image with a 1bpp mask
  void     pushMaskedImage(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, bool swapBytes, uint16_t *img, uint8_t *mask);

  // Colour conversion
           // Convert 8 bit red, green and blue to 16 bits
  uint16_t color565(uint8_t red, uint8_t green, uint8_t blue);

           // Convert 8 bit colour to 16 bits
  uint16_t color8to16(uint8_t color332);
           // Convert 16 bit colour to 8 bits
  uint8_t  color16to8(uint16_t color565);

           // Convert 16 bit colour to/from 24 bit, R+G+B concatenated into LS 24 bits
  uint32_t color16to24(uint16_t color565);
  uint32_t color24to16(uint32_t color888);

           // Alpha blend 2 colours, see generic "alphaBlend_Test" example
           // alpha =   0 = 100% background colour
           // alpha = 255 = 100% foreground colour
  uint16_t alphaBlend(uint8_t alpha, uint16_t fgc, uint16_t bgc);
           // 16 bit colour alphaBlend with alpha dither (dither reduces colour banding)
  uint16_t alphaBlend(uint8_t alpha, uint16_t fgc, uint16_t bgc, uint8_t dither);
           // 24 bit colour alphaBlend with optional alpha dither
  uint32_t alphaBlend24(uint8_t alpha, uint32_t fgc, uint32_t bgc, uint8_t dither = 0);

 //--------------------------------------- private ------------------------------------//
 private:
           // Smooth graphics helper
  uint8_t  sqrt_fraction(uint32_t num);

           // Helper function: calculate distance of a point from a finite length line between two points
  float    wedgeLineDistance(float pax, float pay, float bax, float bay, float dr);
           // Display variant settings

  uint8_t  tabcolor,                   // ST7735 screen protector "tab" colour (now invalid)
           colstart = 0, rowstart = 0; // Screen display area to CGRAM area coordinate offsets

}; // End of class TFT_GFX

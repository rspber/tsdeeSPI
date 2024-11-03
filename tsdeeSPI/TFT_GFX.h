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

#define TFT_ESPI_VERSION "2.5.43"

#include "TFT_eeSPI.h"

/***************************************************************************************
**                         Section 1: Load required header files
***************************************************************************************/

/***************************************************************************************
**                         Section 2: Load library and processor specific header files
***************************************************************************************/

#include <t_cursor.h>
#include <t_gradient.h>

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
                   drawLine(clip_t& clip, int32_t xs, int32_t ys, int32_t xe, int32_t ye, rgb_t color),
                   drawFastVLine(clip_t& clip, int32_t x, int32_t y, int32_t h, rgb_t color),
                   drawFastHLine(clip_t& clip, int32_t x, int32_t y, int32_t w, rgb_t color),
                   fillRect(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, rgb_t color);

  // Graphics drawing
  void
           drawRect(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, rgb_t color),
           drawRoundRect(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, rgb_t color),
           fillRoundRect(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, rgb_t color);

  void     fillRectVGradient(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, rgb_t color1, rgb_t color2);
  void     fillRectHGradient(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, rgb_t color1, rgb_t color2);

  void     drawCircle(clip_t& clip, int32_t x, int32_t y, int32_t r, rgb_t color),
           drawCircleHelper(clip_t& clip, int32_t x, int32_t y, int32_t r, uint8_t cornername, rgb_t color),
           fillCircle(clip_t& clip, int32_t x, int32_t y, int32_t r, rgb_t color),
           fillCircleHelper(clip_t& clip, int32_t x, int32_t y, int32_t r, uint8_t cornername, int32_t delta, rgb_t color),

           drawEllipse(clip_t& clip, int16_t x, int16_t y, int32_t rx, int32_t ry, uint16_t color),
           fillEllipse(clip_t& clip, int16_t x, int16_t y, int32_t rx, int32_t ry, uint16_t color),

           //                 Corner 1               Corner 2               Corner 3
           drawTriangle(clip_t& clip, int32_t x1,int32_t y1, int32_t x2,int32_t y2, int32_t x3,int32_t y3, rgb_t color),
           fillTriangle(clip_t& clip, int32_t x1,int32_t y1, int32_t x2,int32_t y2, int32_t x3,int32_t y3, rgb_t color);


  // Smooth (anti-aliased) graphics drawing
           // Draw a pixel blended with the background pixel colour (bg_color) specified,  return blended colour
           // If the bg_color is not specified, the background pixel colour will be read from TFT or sprite
  uint16_t drawPixel(clip_t& clip, int32_t x, int32_t y, rgb_t color, uint8_t alpha, rgb_t bg_color = TFT_WHITE);

  using TFT_eeSPI::drawPixel;

           // Draw an anti-aliased (smooth) arc between start and end angles. Arc ends are anti-aliased.
           // By default the arc is drawn with square ends unless the "roundEnds" parameter is included and set true
           // Angle = 0 is at 6 o'clock position, 90 at 9 o'clock etc. The angles must be in range 0-360 or they will be clipped to these limits
           // The start angle may be larger than the end angle. Arcs are always drawn clockwise from the start angle.
  void     drawSmoothArc(clip_t& clip, int32_t x, int32_t y, int32_t r, int32_t ir, uint32_t startAngle, uint32_t endAngle, rgb_t fg_color, rgb_t bg_color, bool roundEnds = false);
           // As per "drawSmoothArc" except the ends of the arc are NOT anti-aliased, this facilitates dynamic arc length changes with
           // arc segments and ensures clean segment joints.
           // The sides of the arc are anti-aliased by default. If smoothArc is false sides will NOT be anti-aliased
  void     drawArc(clip_t& clip, int32_t x, int32_t y, int32_t r, int32_t ir, uint32_t startAngle, uint32_t endAngle, rgb_t fg_color, rgb_t bg_color, bool smoothArc = true);

           // Draw an anti-aliased filled circle at x, y with radius r
           // Note: The thickness of line is 3 pixels to reduce the visible "braiding" effect of anti-aliasing narrow lines
           //       this means the inner anti-alias zone is always at r-1 and the outer zone at r+1
  void     drawSmoothCircle(clip_t& clip, int32_t x, int32_t y, int32_t r, rgb_t fg_color, rgb_t bg_color);

           // Draw an anti-aliased filled circle at x, y with radius r
           // If bg_color is not included the background pixel colour will be read from TFT or sprite
  void     fillSmoothCircle(clip_t& clip, int32_t x, int32_t y, int32_t r, rgb_t color, rgb_t bg_color = TFT_WHITE);

           // Draw a rounded rectangle that has a line thickness of r-ir+1 and bounding box defined by x,y and w,h
           // The outer corner radius is r, inner corner radius is ir
           // The inside and outside of the border are anti-aliased
  void     drawSmoothRoundRect(clip_t& clip, int32_t x, int32_t y, int32_t r, int32_t ir, int32_t w, int32_t h, rgb_t fg_color, rgb_t bg_color = TFT_WHITE, uint8_t quadrants = 0xF);

           // Draw a filled rounded rectangle , corner radius r and bounding box defined by x,y and w,h
  void     fillSmoothRoundRect(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, rgb_t color, rgb_t bg_color = TFT_WHITE);

           // Draw a small anti-aliased filled circle at ax,ay with radius r (uses drawWideLine)
           // If bg_color is not included the background pixel colour will be read from TFT or sprite
  void     drawSpot(clip_t& clip, float ax, float ay, float r, rgb_t fg_color, rgb_t bg_color = TFT_WHITE);

           // Draw an anti-aliased wide line from ax,ay to bx,by width wd with radiused ends (radius is wd/2)
           // If bg_color is not included the background pixel colour will be read from TFT or sprite
  void     drawWideLine(clip_t& clip, float ax, float ay, float bx, float by, float wd, rgb_t fg_color, rgb_t bg_color = TFT_WHITE);

           // Draw an anti-aliased wide line from ax,ay to bx,by with different width at each end aw, bw and with radiused ends
           // If bg_color is not included the background pixel colour will be read from TFT or sprite
  void     drawWedgeLine(clip_t& clip, float ax, float ay, float bx, float by, float aw, float bw, rgb_t fg_color, rgb_t bg_color = TFT_WHITE);


  // Image rendering
           // Draw bitmap
  void     drawBitmap(clip_t& clip, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, rgb_t fgcolor),
           drawBitmap(clip_t& clip, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, rgb_t fgcolor, rgb_t bgcolor),
           drawXBitmap(clip_t& clip, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, rgb_t fgcolor),
           drawXBitmap(clip_t& clip, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, rgb_t fgcolor, rgb_t bgcolor),
           setBitmapColor(uint16_t fgcolor, uint16_t bgcolor); // Define the 2 colours for 1bpp sprites

           // Write a block of pixels to the screen which have been read by readRect()
  void     pushRect(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data);

           // These are used to render images or sprites stored in RAM arrays (used by Sprite class for 16bpp Sprites)
  virtual void pushImage16(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data);
  virtual void pushImage16(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data, rgb_t transparent);

           // These are used to render images stored in FLASH (PROGMEM)
  void     pushImage16(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data, rgb_t transparent);
  void     pushImage16(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data);

           // These are used by Sprite class pushSprite() member function for 1, 4 and 8 bits per pixel (bpp) colours
           // They are not intended to be used with user sketches (but could be)
           // Set bpp8 true for 8bpp sprites, false otherwise. The cmap pointer must be specified for 4bpp
  void     pushImage16(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, uint8_t  *data, bool bpp8 = true, uint16_t *cmap = nullptr);
  void     pushImage16(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, uint8_t  *data, uint8_t  transparent, bool bpp8 = true, uint16_t *cmap = nullptr);
           // FLASH version
  void     pushImage16(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, const uint8_t *data, bool bpp8, uint16_t *cmap = nullptr);

           // Render a 16-bit colour image with a 1bpp mask
  void     pushMaskedImage16(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *img, uint8_t *mask);

  uint32_t _bitmap_fg, _bitmap_bg;           // Bitmap foreground (bit=1) and background (bit=0) colours

 //--------------------------------------- private ------------------------------------//
 protected:
           // Smooth graphics helper
  uint8_t  sqrt_fraction(uint32_t num);

           // Helper function: calculate distance of a point from a finite length line between two points
  float    wedgeLineDistance(float pax, float pay, float bax, float bay, float dr);

/***************************************************************************************
**                         Section 9: TFT_eSPI class conditional extensions
***************************************************************************************/

}; // End of class TFT_GFX


           // Alpha blend 2 colours, see generic "alphaBlend_Test" example
           // alpha =   0 = 100% background colour
           // alpha = 255 = 100% foreground colour

/***************************************************************************************
** Function name:           alphaBlend
** Description:             Blend 24bit foreground and background
*************************************************************************************x*/
inline rgb_t alphaBlend(uint8_t alpha, rgb_t fgc, rgb_t bgc)
{
  // Split out and blend 5 bit red and blue channels
  uint32_t rxb = bgc & 0xFF00FF;
  rxb += ((fgc & 0xFF00FF) - rxb) * (alpha >> 2) >> 6;
  // Split out and blend 6-bit green channel
  uint32_t xgx = bgc & 0x00FF00;
  xgx += ((fgc & 0x00FF00) - xgx) * alpha >> 8;
  // Recombine channels
  return (rxb & 0xFF00FF) | (xgx & 0x00FF00);
}

/***************************************************************************************
** Function name:           alphaBlend
** Description:             Blend 16bit foreground and background with dither
*************************************************************************************x*/
           // 16-bit colour alphaBlend with alpha dither (dither reduces colour banding)
inline rgb_t alphaBlend(uint8_t alpha, rgb_t fgc, rgb_t bgc, uint8_t dither)
{
  if (dither) {
    int16_t alphaDither = (int16_t)alpha - dither + random(2*dither+1); // +/-4 randomised
    alpha = (uint8_t)alphaDither;
    if (alphaDither <  0) alpha = 0;
    if (alphaDither >255) alpha = 255;
  }

  return alphaBlend(alpha, fgc, bgc);
}

/***************************************************************************************
** Function name:           alphaBlend
** Description:             Blend 24bit foreground and background with optional dither
*************************************************************************************x*/
           // 24-bit colour alphaBlend with optional alpha dither
inline rgb_t alphaBlend24(uint8_t alpha, rgb_t fgc, rgb_t bgc, uint8_t dither = 0)
{

  if (dither) {
    int16_t alphaDither = (int16_t)alpha - dither + random(2*dither+1); // +/-dither randomised
    alpha = (uint8_t)alphaDither;
    if (alphaDither <  0) alpha = 0;
    if (alphaDither >255) alpha = 255;
  }

  uint32_t rxx = bgc & 0xFF0000;
  rxx += ((fgc & 0xFF0000) - rxx) * alpha >> 8;
  uint32_t xgx = bgc & 0x00FF00;
  xgx += ((fgc & 0x00FF00) - xgx) * alpha >> 8;
  uint32_t xxb = bgc & 0x0000FF;
  xxb += ((fgc & 0x0000FF) - xxb) * alpha >> 8;
  return (rxx & 0xFF0000) | (xgx & 0x00FF00) | (xxb & 0x0000FF);
}



/***************************************************************************************
**                         Section 10: Additional extension classes
***************************************************************************************/

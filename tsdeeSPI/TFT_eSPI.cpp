/*
  Top level

  Copyright (c) 2024, rspber (https://github.com/rspber)

  Based on: TFT_eSPI

  Originally notes below:
*/

/***************************************************
  Arduino TFT graphics library targeted at 32-bit
  processors such as ESP32, ESP8266 and STM32.

  This is a stand-alone library that contains the
  hardware driver, the graphics functions and the
  proportional fonts.

  The larger fonts are Run Length Encoded to reduce their
  size.

  Created by Bodmer 2/12/16
  Last update by Bodmer 20/03/20
 ****************************************************/

#include "TFT_eSPI.h"

/***************************************************************************************
** Function name:           setViewport
** Description:             Set the clipping region for the TFT screen
***************************************************************************************/
void TFT_eSPI::setViewport(int32_t x, int32_t y, int32_t w, int32_t h, bool vpDatum)
{
  // Viewport metrics (not clipped)
  _clip.xDatum  = x; // Datum x position in screen coordinates
  _clip.yDatum  = y; // Datum y position in screen coordinates
  _xWidth  = w; // Viewport width
  _yHeight = h; // Viewport height

  // Full size default viewport
  _vpDatum = false; // Datum is at top left corner of screen (true = top left of viewport)
  _clip.x1 = 0;         // Viewport top left corner x coordinate
  _clip.y1 = 0;         // Viewport top left corner y coordinate
  _clip.x2 = width();   // Equivalent of TFT width  (Nb: viewport right edge coord + 1)
  _clip.y2 = height();  // Equivalent of TFT height (Nb: viewport bottom edge coord + 1)

  // Clip viewport to screen area
  if (x<0) { w += x; x = 0; }
  if (y<0) { h += y; y = 0; }
  if ((x + w) > width() ) { w = width()  - x; }
  if ((y + h) > height() ) { h = height() - y; }

  //Serial.print(" x=");Serial.print( x);Serial.print(", y=");Serial.print( y);
  //Serial.print(", w=");Serial.print(w);Serial.print(", h=");Serial.println(h);

  if (!vpDatum)
  {
    _clip.xDatum = 0; // Reset to top left of screen if not using a viewport datum
    _clip.yDatum = 0;
    _xWidth  = width();
    _yHeight = height();
  }

  // Store the clipped screen viewport metrics and datum position
  _clip.x1 = x;
  _clip.y1 = y;
  _clip.x2 = x + w;
  _clip.y2 = y + h;
  _vpDatum = vpDatum;

  //Serial.print(" _clip.xDatum=");Serial.print( _clip.xDatum);Serial.print(", _clip.yDatum=");Serial.print( _clip.yDatum);
  //Serial.print(", _xWidth=");Serial.print(_xWidth);Serial.print(", _yHeight=");Serial.println(_yHeight);

  //Serial.print(" _clip.x1=");Serial.print( _clip.x1);Serial.print(", _clip.y1=");Serial.print( _clip.y1);
  //Serial.print(", _clip.x2=");Serial.print(_clip.x2);Serial.print(", _clip.y2=");Serial.println(_clip.y2);
}

/***************************************************************************************
** Function name:           checkViewport
** Description:             Check if any part of specified area is visible in viewport
***************************************************************************************/
// Note: Setting w and h to 1 will check if coordinate x,y is in area
void checkViewport_(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, bool& ret)
{
  ret = false;
  PI_CLIP;
  ret = true;
}

bool TFT_eSPI::checkViewport(int32_t x, int32_t y, int32_t w, int32_t h)
{
  bool ret;
  checkViewport_(_clip, x, y, w, h, ret);
  return ret;
}

/***************************************************************************************
** Function name:           resetViewport
** Description:             Reset viewport to whole TFT screen, datum at 0,0
***************************************************************************************/
void TFT_eSPI::resetViewport(void)
{
  // Reset viewport to the whole screen (or sprite) area
  _vpDatum = false;
  _clip.xDatum = 0;
  _clip.yDatum = 0;
  _clip.x1 = 0;
  _clip.y1 = 0;
  _clip.x2 = width();
  _clip.y2 = height();
  _xWidth  = width();
  _yHeight = height();
}

/***************************************************************************************
** Function name:           getViewportX
** Description:             Get x position of the viewport datum
***************************************************************************************/
int32_t  TFT_eSPI::getViewportX(void)
{
  return _clip.xDatum;
}

/***************************************************************************************
** Function name:           getViewportY
** Description:             Get y position of the viewport datum
***************************************************************************************/
int32_t  TFT_eSPI::getViewportY(void)
{
  return _clip.yDatum;
}

/***************************************************************************************
** Function name:           getViewportWidth
** Description:             Get width of the viewport
***************************************************************************************/
int32_t TFT_eSPI::getViewportWidth(void)
{
  return _xWidth;
}

/***************************************************************************************
** Function name:           getViewportHeight
** Description:             Get height of the viewport
***************************************************************************************/
int32_t TFT_eSPI::getViewportHeight(void)
{
  return _yHeight;
}

/***************************************************************************************
** Function name:           getViewportDatum
** Description:             Get datum flag of the viewport (true = viewport corner)
***************************************************************************************/
bool  TFT_eSPI::getViewportDatum(void)
{
  return _vpDatum;
}

/***************************************************************************************
** Function name:           frameViewport
** Description:             Draw a frame inside or outside the viewport of width w
***************************************************************************************/
void TFT_eSPI::frameViewport(uint16_t color, int32_t w)
{
  // Save datum position
  bool _dT = _vpDatum;

  // If w is positive the frame is drawn inside the viewport
  // a large positive width will clear the screen inside the viewport
  if (w>0)
  {
    // Set vpDatum true to simplify coordinate derivation
    _vpDatum = true;
    fillRect(0, 0, _clip.x2 - _clip.x1, w, color);                // Top
    fillRect(0, w, w, _clip.y2 - _clip.y1 - w - w, color);        // Left
    fillRect(_xWidth - w, w, w, _yHeight - w - w, color); // Right
    fillRect(0, _yHeight - w, _xWidth, w, color);         // Bottom
  }
  else
  // If w is negative the frame is drawn outside the viewport
  // a large negative width will clear the screen outside the viewport
  {
    w = -w;

    // Save old values
    int32_t _xT = _clip.x1; _clip.x1 = 0;
    int32_t _yT = _clip.y1; _clip.y1 = 0;
    int32_t _wT = _clip.x2;
    int32_t _hT = _clip.y2;

    // Set vpDatum false so frame can be drawn outside window
    _vpDatum = false; // When false the full width and height is accessed
    _clip.y2 = height();
    _clip.x2 = width();

    // Draw frame
    fillRect(_xT - w - _clip.xDatum, _yT - w - _clip.yDatum, _wT - _xT + w + w, w, color); // Top
    fillRect(_xT - w - _clip.xDatum, _yT - _clip.yDatum, w, _hT - _yT, color);             // Left
    fillRect(_wT - _clip.xDatum, _yT - _clip.yDatum, w, _hT - _yT, color);                 // Right
    fillRect(_xT - w - _clip.xDatum, _hT - _clip.yDatum, _wT - _xT + w + w, w, color);     // Bottom

    // Restore old values
    _clip.x1 = _xT;
    _clip.y1 = _yT;
    _clip.x2 = _wT;
    _clip.y2 = _hT;
  }

  // Restore vpDatum
  _vpDatum = _dT;
}


/***************************************************************************************
** Function name:           TFT_eSPI
** Description:             Constructor , we must use hardware SPI pins
***************************************************************************************/
TFT_eSPI::TFT_eSPI() : TFT_Print()
{
  // Reset the viewport to the whole screen
  resetViewport();

  _xPivot = 0;
  _yPivot = 0;
}


/***************************************************************************************
** Function name:           setOrigin
** Description:             Set graphics origin to position x,y wrt to top left corner
***************************************************************************************/
//Note: setRotation, setViewport and resetViewport will revert origin to top left
void TFT_eSPI::setOrigin(int32_t x, int32_t y)
{
  _clip.xDatum = x;
  _clip.yDatum = y;
}


/***************************************************************************************
** Function name:           getOriginX
** Description:             Set graphics origin to position x
***************************************************************************************/
int32_t TFT_eSPI::getOriginX(void)
{
  return _clip.xDatum;
}


/***************************************************************************************
** Function name:           getOriginY
** Description:             Set graphics origin to position y
***************************************************************************************/
int32_t TFT_eSPI::getOriginY(void)
{
  return _clip.yDatum;
}


/***************************************************************************************
** Function name:           read pixel (for SPI Interface II i.e. IM [3:0] = "1101")
** Description:             Read 565 pixel colours from a pixel
***************************************************************************************/
uint16_t TFT_eSPI::readPixel(int32_t x0, int32_t y0)
{
  return readPixel(_clip, x0, y0);
}


/***************************************************************************************
** Function name:           read rectangle (for SPI Interface II i.e. IM [3:0] = "1101")
** Description:             Read 565 pixel colours from a defined area
***************************************************************************************/
void TFT_eSPI::readRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data)
{
  readRect(_clip, x, y, w, h, data);
}


/***************************************************************************************
** Function name:           push rectangle
** Description:             push 565 pixel colours into a defined area
***************************************************************************************/
void TFT_eSPI::pushRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data)
{
  pushRect(_clip, x, y, w, h, data);
}


/***************************************************************************************
** Function name:           pushImage
** Description:             plot 16-bit colour sprite or image onto TFT
***************************************************************************************/
void TFT_eSPI::pushImage16(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data)
{
  pushImage16(_clip, x, y, w, h, data);
}

/***************************************************************************************
** Function name:           pushImage
** Description:             plot 16-bit sprite or image with 1 colour being transparent
***************************************************************************************/
void TFT_eSPI::pushImage16(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data, rgb_t transp)
{
  pushImage16(_clip, x, y, w, h, data, transp);
}


/***************************************************************************************
** Function name:           pushImage - for FLASH (PROGMEM) stored images
** Description:             plot 16-bit image
***************************************************************************************/
void TFT_eSPI::pushImage16(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data)
{
  pushImage16(_clip, x, y, w, h, data);
}

/***************************************************************************************
** Function name:           pushImage - for FLASH (PROGMEM) stored images
** Description:             plot 16-bit image with 1 colour being transparent
***************************************************************************************/
void TFT_eSPI::pushImage16(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data, rgb_t transp)
{
  pushImage16(_clip, x, y, w, h, data, transp);
}

/***************************************************************************************
** Function name:           pushImage
** Description:             plot 8-bit or 4-bit or 1 bit image or sprite using a line buffer
***************************************************************************************/
void TFT_eSPI::pushImage16(int32_t x, int32_t y, int32_t w, int32_t h, const uint8_t *data, bool bpp8, cmap_t& cmap)
{
  pushImage16(_clip, x, y, w, h, data, bpp8, cmap);
}


/***************************************************************************************
** Function name:           pushImage
** Description:             plot 8-bit or 4-bit or 1 bit image or sprite using a line buffer
***************************************************************************************/
void TFT_eSPI::pushImage16(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t *data, bool bpp8, cmap_t& cmap)
{
  pushImage16(_clip, x, y, w, h, data, bpp8, cmap);
}


/***************************************************************************************
** Function name:           pushImage
** Description:             plot 8 or 4 or 1 bit image or sprite with a transparent colour
***************************************************************************************/
void TFT_eSPI::pushImage16(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t *data, rgb_t transp, bool bpp8, cmap_t& cmap)
{
  pushImage16(_clip, x, y, w, h, data, transp, bpp8, cmap);
}

/***************************************************************************************
** Function name:           pushMaskedImage
** Description:             Render a 16-bit colour image to TFT with a 1bpp mask
***************************************************************************************/
// Can be used with a 16bpp sprite and a 1bpp sprite for the mask
void TFT_eSPI::pushMaskedImage16(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *img, uint8_t *mask)
{
  pushMaskedImage16(_clip, x, y, w, h, img, mask);
}


/***************************************************************************************
** Function name:           readRectRGB
** Description:             tsdeeSPI
***************************************************************************************/
void TFT_eSPI::readRectRGB(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t *data)
{
  readRectRGB(_clip, x, y, w, h, data);
}

/***************************************************************************************
** Function name:           pushImageDMA
** Description:             tsdeeSPI
***************************************************************************************/
void TFT_eSPI::pushImageDMA16(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t* data, uint16_t* buffer)
{
  pushImageDMA16(_clip, x, y, w, h, data, buffer);
}


/***************************************************************************************
** Function name:           drawCircle
** Description:             Draw a circle outline
***************************************************************************************/
// Optimised midpoint circle algorithm
void TFT_eSPI::drawCircle(int32_t x0, int32_t y0, int32_t r, rgb_t color)
{
  drawCircle(_clip, x0, y0, r, color);
}


/***************************************************************************************
** Function name:           drawCircleHelper
** Description:             Support function for drawRoundRect()
***************************************************************************************/
void TFT_eSPI::drawCircleHelper( int32_t x0, int32_t y0, int32_t rr, uint8_t cornername, rgb_t color)
{
  drawCircleHelper(_clip, x0, y0, rr, cornername, color);
}

/***************************************************************************************
** Function name:           fillCircle
** Description:             draw a filled circle
***************************************************************************************/
// Optimised midpoint circle algorithm, changed to horizontal lines (faster in sprites)
// Improved algorithm avoids repetition of lines
void TFT_eSPI::fillCircle(int32_t x0, int32_t y0, int32_t r, rgb_t color)
{
  fillCircle(_clip, x0, y0, r, color);
}

/***************************************************************************************
** Function name:           fillCircleHelper
** Description:             Support function for fillRoundRect()
***************************************************************************************/
// Support drawing roundrects, changed to horizontal lines (faster in sprites)
void TFT_eSPI::fillCircleHelper(int32_t x0, int32_t y0, int32_t r, uint8_t cornername, int32_t delta, rgb_t color)
{
  fillCircleHelper(_clip, x0, y0, r, cornername, delta, color);
}


/***************************************************************************************
** Function name:           drawEllipse
** Description:             Draw a ellipse outline
***************************************************************************************/
void TFT_eSPI::drawEllipse(int16_t x0, int16_t y0, int32_t rx, int32_t ry, uint16_t color)
{
  drawEllipse(_clip, x0, y0, rx, ry, color);
}


/***************************************************************************************
** Function name:           fillEllipse
** Description:             draw a filled ellipse
***************************************************************************************/
void TFT_eSPI::fillEllipse(int16_t x0, int16_t y0, int32_t rx, int32_t ry, uint16_t color)
{
  fillEllipse(_clip, x0, y0, rx, ry, color);
}


/***************************************************************************************
** Function name:           fillScreen
** Description:             Clear the screen to defined colour
***************************************************************************************/
void TFT_eSPI::fillScreen(rgb_t color)
{
  fillRect(0, 0, _clip.xWidth, _clip.yHeight, color);
}


/***************************************************************************************
** Function name:           drawRect
** Description:             Draw a rectangle outline
***************************************************************************************/
// Draw a rectangle
void TFT_eSPI::drawRect(int32_t x, int32_t y, int32_t w, int32_t h, rgb_t color)
{
  drawRect(_clip, x, y, w, h, color);
}


/***************************************************************************************
** Function name:           drawRoundRect
** Description:             Draw a rounded corner rectangle outline
***************************************************************************************/
void TFT_eSPI::drawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, rgb_t color)
{
  drawRoundRect(_clip, x, y, w, h, radius, color);
}


/***************************************************************************************
** Function name:           fillRoundRect
** Description:             Draw a rounded corner filled rectangle
***************************************************************************************/
// Fill a rounded rectangle, changed to horizontal lines (faster in sprites)
void TFT_eSPI::fillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, rgb_t color)
{
  fillRoundRect(_clip, x, y, w, h, r, color);
}


/***************************************************************************************
** Function name:           drawTriangle
** Description:             Draw a triangle outline using 3 arbitrary points
***************************************************************************************/
// Draw a triangle
void TFT_eSPI::drawTriangle(int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, rgb_t color)
{
  drawTriangle(_clip, x0, y0, x1, y1, x2, y2, color);
}


/***************************************************************************************
** Function name:           fillTriangle
** Description:             Draw a filled triangle using 3 arbitrary points
***************************************************************************************/
// Fill a triangle - original Adafruit function works well and code footprint is small
void TFT_eSPI::fillTriangle ( int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, rgb_t color)
{
  fillTriangle(_clip, x0, y0, x1, y1, x2, y2, color);
}


/***************************************************************************************
** Function name:           drawBitmap
** Description:             Draw an image stored in an array on the TFT
***************************************************************************************/
void TFT_eSPI::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{
  drawBitmap(_clip, x, y, bitmap, w, h, color);
}


/***************************************************************************************
** Function name:           drawBitmap
** Description:             Draw an image stored in an array on the TFT
***************************************************************************************/
void TFT_eSPI::drawBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t fgcolor, uint16_t bgcolor)
{
  drawBitmap(_clip, x, y, bitmap, w, h, fgcolor, bgcolor);
}

/***************************************************************************************
** Function name:           drawXBitmap
** Description:             Draw an image stored in an XBM array onto the TFT
***************************************************************************************/
void TFT_eSPI::drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{
  drawXBitmap(_clip, x, y, bitmap, w, h, color);
}


/***************************************************************************************
** Function name:           drawXBitmap
** Description:             Draw an XBM image with foreground and background colors
***************************************************************************************/
void TFT_eSPI::drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bgcolor)
{
  drawXBitmap(_clip, x, y, bitmap, w, h, color, bgcolor);
}


/***************************************************************************************
** Function name:           setPivot
** Description:             Set the pivot point on the TFT
*************************************************************************************x*/
void TFT_eSPI::setPivot(int16_t x, int16_t y)
{
  _xPivot = x;
  _yPivot = y;
}


/***************************************************************************************
** Function name:           getPivotX
** Description:             Get the x pivot position
***************************************************************************************/
int16_t TFT_eSPI::getPivotX(void)
{
  return _xPivot;
}


/***************************************************************************************
** Function name:           getPivotY
** Description:             Get the y pivot position
***************************************************************************************/
int16_t TFT_eSPI::getPivotY(void)
{
  return _yPivot;
}


/***************************************************************************************
** Function name:           width
** Description:             Return the pixel width of display (per current rotation)
***************************************************************************************/
// Return the size of the display (per current rotation)
int16_t TFT_eSPI::width(void)
{
  if (_vpDatum) return _xWidth;
  return _clip.xWidth;
}


/***************************************************************************************
** Function name:           height
** Description:             Return the pixel height of display (per current rotation)
***************************************************************************************/
int16_t TFT_eSPI::height(void)
{
  if (_vpDatum) return _yHeight;
  return _clip.yHeight;
}


/***************************************************************************************
** Function name:           drawPixel
** Description:             push a single pixel at an arbitrary position
***************************************************************************************/
void TFT_eSPI::drawPixel(int32_t x, int32_t y, rgb_t color)
{
  drawPixel(_clip, x, y, color);
}

/***************************************************************************************
** Function name:           drawLine
** Description:             draw a line between 2 arbitrary points
***************************************************************************************/
void TFT_eSPI::drawLine(int32_t x0, int32_t y0, int32_t x1, int32_t y1, rgb_t color)
{
  drawLine(_clip, x0, y0, x1, y1, color);
}


/***************************************************************************************
** Function name:           drawPixel (alpha blended)
** Description:             Draw a pixel blended with the screen or bg pixel colour
***************************************************************************************/
uint16_t TFT_eSPI::drawPixel(int32_t x, int32_t y, rgb_t color, uint8_t alpha, rgb_t bg_color)
{
  if (bg_color == 0x00FFFFFF) bg_color = readPixel(x, y);
  color = alphaBlend(alpha, color, bg_color);
  drawPixel(x, y, color);
  return color;
}


/***************************************************************************************
** Function name:           drawSmoothArc
** Description:             Draw a smooth arc clockwise from 6 o'clock
***************************************************************************************/
void TFT_eSPI::drawSmoothArc(int32_t x, int32_t y, int32_t r, int32_t ir, uint32_t startAngle, uint32_t endAngle, rgb_t fg_color, rgb_t bg_color, bool roundEnds)
// Centre at x,y
// r = arc outer radius, ir = arc inner radius. Inclusive so arc thickness = r - ir + 1
// Angles in range 0-360
// Arc foreground colour anti-aliased with background colour at edges
// anti-aliased roundEnd is optional, default is anti-aliased straight end
// Note: rounded ends extend the arc angle so can overlap, user sketch to manage this.
{
  drawSmoothArc(_clip, x, y, r, ir, startAngle, endAngle, fg_color, bg_color, roundEnds);
}

/***************************************************************************************
** Function name:           drawArc
** Description:             Draw an arc clockwise from 6 o'clock position
***************************************************************************************/
// Centre at x,y
// r = arc outer radius, ir = arc inner radius. Inclusive, so arc thickness = r-ir+1
// Angles MUST be in range 0-360
// Arc foreground fg_color anti-aliased with background colour along sides
// smooth is optional, default is true, smooth=false means no antialiasing
// Note: Arc ends are not anti-aliased (use drawSmoothArc instead for that)
void TFT_eSPI::drawArc(int32_t x, int32_t y, int32_t r, int32_t ir,
                       uint32_t startAngle, uint32_t endAngle,
                       rgb_t fg_color, rgb_t bg_color,
                       bool smooth)
{
  drawArc(_clip, x, y, r, ir, startAngle, endAngle, fg_color, bg_color, smooth);
}

/***************************************************************************************
** Function name:           drawSmoothCircle
** Description:             Draw a smooth circle
***************************************************************************************/
// To have effective anti-aliasing the circle will be 3 pixels thick
void TFT_eSPI::drawSmoothCircle(int32_t x, int32_t y, int32_t r, rgb_t fg_color, rgb_t bg_color)
{
  drawSmoothCircle(_clip, x, y, r, fg_color, bg_color);
}

/***************************************************************************************
** Function name:           fillSmoothCircle
** Description:             Draw a filled anti-aliased circle
***************************************************************************************/
void TFT_eSPI::fillSmoothCircle(int32_t x, int32_t y, int32_t r, rgb_t color, rgb_t bg_color)
{
  fillSmoothCircle(_clip, x, y, r, color, bg_color);
}


/***************************************************************************************
** Function name:           drawSmoothRoundRect
** Description:             Draw a rounded rectangle
***************************************************************************************/
// x,y is top left corner of bounding box for a complete rounded rectangle
// r = arc outer corner radius, ir = arc inner radius. Arc thickness = r-ir+1
// w and h are width and height of the bounding rectangle
// If w and h are < radius (e.g. 0,0) a circle will be drawn with centre at x+r,y+r
// Arc foreground fg_color anti-aliased with background colour at edges
// A subset of corners can be drawn by specifying a quadrants mask. A bit set in the
// mask means draw that quadrant (all are drawn if parameter missing):
//   0x1 | 0x2
//    ---¦---    Arc quadrant mask select bits (as in drawCircleHelper fn)
//   0x8 | 0x4
void TFT_eSPI::drawSmoothRoundRect(int32_t x, int32_t y, int32_t r, int32_t ir, int32_t w, int32_t h, rgb_t fg_color, rgb_t bg_color, uint8_t quadrants)
{
  drawSmoothRoundRect(_clip, x, y, r, ir, w, h, fg_color, bg_color, quadrants);
}

/***************************************************************************************
** Function name:           fillSmoothRoundRect
** Description:             Draw a filled anti-aliased rounded corner rectangle
***************************************************************************************/
void TFT_eSPI::fillSmoothRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, rgb_t color, rgb_t bg_color)
{
  fillSmoothRoundRect(_clip, x, y, w, h, r, color, bg_color);
}

/***************************************************************************************
** Function name:           drawSpot - maths intensive, so for small filled circles
** Description:             Draw an anti-aliased filled circle at ax,ay with radius r
***************************************************************************************/
// Coordinates are floating point to achieve sub-pixel positioning
void TFT_eSPI::drawSpot(float ax, float ay, float r, rgb_t fg_color, rgb_t bg_color)
{
  // Filled circle can be created by the wide line function with zero line length
  drawWedgeLine(_clip, ax, ay, ax, ay, r, r, fg_color, bg_color);
}

/***************************************************************************************
** Function name:           drawWideLine - background colour specified or pixel read
** Description:             draw an anti-aliased line with rounded ends, width wd
***************************************************************************************/
void TFT_eSPI::drawWideLine(float ax, float ay, float bx, float by, float wd, rgb_t fg_color, rgb_t bg_color)
{
  drawWedgeLine(_clip, ax, ay, bx, by, wd/2.0, wd/2.0, fg_color, bg_color);
}

/***************************************************************************************
** Function name:           drawWedgeLine - background colour specified or pixel read
** Description:             draw an anti-aliased line with different width radiused ends
***************************************************************************************/
void TFT_eSPI::drawWedgeLine(float ax, float ay, float bx, float by, float ar, float br, rgb_t fg_color, rgb_t bg_color)
{
  drawWedgeLine(_clip, ax, ay, bx, by, ar, br, fg_color, bg_color);
}


/***************************************************************************************
** Function name:           drawFastVLine
** Description:             draw a vertical line
***************************************************************************************/
void TFT_eSPI::drawFastVLine(int32_t x, int32_t y, int32_t h, rgb_t color)
{
  drawFastVLine(_clip, x, y, h, color);
}


/***************************************************************************************
** Function name:           drawFastHLine
** Description:             draw a horizontal line
***************************************************************************************/
void TFT_eSPI::drawFastHLine(int32_t x, int32_t y, int32_t w, rgb_t color)
{
  drawFastHLine(_clip, x, y, w, color);
}


/***************************************************************************************
** Function name:           fillRect
** Description:             draw a filled rectangle
***************************************************************************************/
void TFT_eSPI::fillRect(int32_t x, int32_t y, int32_t w, int32_t h, rgb_t color)
{
  fillRect(_clip, x, y, w, h, color);
}


/***************************************************************************************
** Function name:           fillRectVGradient
** Description:             draw a filled rectangle with a vertical colour gradient
***************************************************************************************/
void TFT_eSPI::fillRectVGradient(int32_t x, int32_t y, int32_t w, int32_t h, rgb_t color1, rgb_t color2)
{
  fillRectVGradient(_clip, x, y, w, h, color1, color2);
}


/***************************************************************************************
** Function name:           fillRectHGradient
** Description:             draw a filled rectangle with a horizontal colour gradient
***************************************************************************************/
void TFT_eSPI::fillRectHGradient(int32_t x, int32_t y, int32_t w, int32_t h, rgb_t color1, rgb_t color2)
{
  fillRectHGradient(_clip, x, y, w, h, color1, color2);
}


/**************************************************************************
** Function name:           setAttribute
** Description:             Sets a control parameter of an attribute
**************************************************************************/
void TFT_eSPI::setAttribute(uint8_t attr_id, uint8_t param) {
    switch (attr_id) {
            break;
        case CP437_SWITCH:
            _cp437 = param;
            break;
        case UTF8_SWITCH:
            _utf8  = param;
            decoderState = 0;
            break;
        case PSRAM_ENABLE:
#if defined (ESP32) && defined (CONFIG_SPIRAM_SUPPORT)
            if (psramFound()) _psram_enable = param; // Enable the use of PSRAM (if available)
            else
#endif
            _psram_enable = false;
            break;
        //case 4: // TBD future feature control
        //    _tbd = param;
        //    break;
    }
}


/**************************************************************************
** Function name:           getAttribute
** Description:             Get value of an attribute (control parameter)
**************************************************************************/
uint8_t TFT_eSPI::getAttribute(uint8_t attr_id) {
    switch (attr_id) {
        case CP437_SWITCH: // ON/OFF control of full CP437 character set
            return _cp437;
        case UTF8_SWITCH: // ON/OFF control of UTF-8 decoding
            return _utf8;
        case PSRAM_ENABLE:
            return _psram_enable;
        //case 3: // TBD future feature control
        //    return _tbd;
        //    break;
    }

    return false;
}


/***************************************************************************************
** Function name:           verifySetupID
** Description:             Compare the ID if USER_SETUP_ID defined in user setup file
***************************************************************************************/
bool TFT_eSPI::verifySetupID(uint32_t id)
{
#if defined (USER_SETUP_ID)
  if (USER_SETUP_ID == id) return true;
#else
  id = id; // Avoid warning
#endif
  return false;
}

/***************************************************************************************
** Function name:           getSetup
** Description:             Get the setup details for diagnostic and sketch access
***************************************************************************************/
void TFT_eSPI::getSetup(setup_t &tft_settings)
{
// tft_settings.version is set in header file

#if defined (USER_SETUP_INFO)
  tft_settings.setup_info = USER_SETUP_INFO;
#else
  tft_settings.setup_info = "NA";
#endif

#if defined (USER_SETUP_ID)
  tft_settings.setup_id = USER_SETUP_ID;
#else
  tft_settings.setup_id = 0;
#endif

#if defined (PROCESSOR_ID)
  tft_settings.esp = PROCESSOR_ID;
#else
  tft_settings.esp = -1;
#endif

#if defined (SUPPORT_TRANSACTIONS)
  tft_settings.trans = true;
#else
  tft_settings.trans = false;
#endif

#if defined (TFT_PARALLEL_8_BIT) || defined(TFT_PARALLEL_16_BIT)
  tft_settings.serial = false;
  tft_settings.tft_spi_freq = 0;
#else
  tft_settings.serial = true;
  tft_settings.tft_spi_freq = SPI_FREQUENCY/100000;
  #ifdef SPI_READ_FREQUENCY
    tft_settings.tft_rd_freq = SPI_READ_FREQUENCY/100000;
  #endif
  #ifndef GENERIC_PROCESSOR
    #ifdef TFT_SPI_PORT
      tft_settings.port = TFT_SPI_PORT;
    #else
      tft_settings.port = 255;
    #endif
  #endif
  #ifdef RP2040_PIO_SPI
    tft_settings.interface = 0x10;
  #else
    tft_settings.interface = 0x0;
  #endif
#endif

#if defined(TFT_SPI_OVERLAP)
  tft_settings.overlap = true;
#else
  tft_settings.overlap = false;
#endif

  tft_settings.tft_driver = TFT_DRIVER;
  tft_settings.tft_width  = _init_width;
  tft_settings.tft_height = _init_height;

#ifdef CGRAM_OFFSET
  tft_settings.r0_x_offset = colstart;
  tft_settings.r0_y_offset = rowstart;
  tft_settings.r1_x_offset = 0;
  tft_settings.r1_y_offset = 0;
  tft_settings.r2_x_offset = 0;
  tft_settings.r2_y_offset = 0;
  tft_settings.r3_x_offset = 0;
  tft_settings.r3_y_offset = 0;
#else
  tft_settings.r0_x_offset = 0;
  tft_settings.r0_y_offset = 0;
  tft_settings.r1_x_offset = 0;
  tft_settings.r1_y_offset = 0;
  tft_settings.r2_x_offset = 0;
  tft_settings.r2_y_offset = 0;
  tft_settings.r3_x_offset = 0;
  tft_settings.r3_y_offset = 0;
#endif

#if defined (TFT_MOSI)
  tft_settings.pin_tft_mosi = TFT_MOSI;
#else
  tft_settings.pin_tft_mosi = -1;
#endif

#if defined (TFT_MISO)
  tft_settings.pin_tft_miso = TFT_MISO;
#else
  tft_settings.pin_tft_miso = -1;
#endif

#if defined (TFT_SCLK)
  tft_settings.pin_tft_clk  = TFT_SCLK;
#else
  tft_settings.pin_tft_clk  = -1;
#endif

#if defined (TFT_CS)
  tft_settings.pin_tft_cs   = TFT_CS;
#else
  tft_settings.pin_tft_cs   = -1;
#endif

#if defined (TFT_DC)
  tft_settings.pin_tft_dc  = TFT_DC;
#else
  tft_settings.pin_tft_dc  = -1;
#endif

#if defined (TFT_RD)
  tft_settings.pin_tft_rd  = TFT_RD;
#else
  tft_settings.pin_tft_rd  = -1;
#endif

#if defined (TFT_WR)
  tft_settings.pin_tft_wr  = TFT_WR;
#else
  tft_settings.pin_tft_wr  = -1;
#endif

#if defined (TFT_RST)
  tft_settings.pin_tft_rst = TFT_RST;
#else
  tft_settings.pin_tft_rst = -1;
#endif

#if defined (TFT_PARALLEL_8_BIT) || defined(TFT_PARALLEL_16_BIT)
  tft_settings.pin_tft_d0 = TFT_D0;
  tft_settings.pin_tft_d1 = TFT_D1;
  tft_settings.pin_tft_d2 = TFT_D2;
  tft_settings.pin_tft_d3 = TFT_D3;
  tft_settings.pin_tft_d4 = TFT_D4;
  tft_settings.pin_tft_d5 = TFT_D5;
  tft_settings.pin_tft_d6 = TFT_D6;
  tft_settings.pin_tft_d7 = TFT_D7;
#else
  tft_settings.pin_tft_d0 = -1;
  tft_settings.pin_tft_d1 = -1;
  tft_settings.pin_tft_d2 = -1;
  tft_settings.pin_tft_d3 = -1;
  tft_settings.pin_tft_d4 = -1;
  tft_settings.pin_tft_d5 = -1;
  tft_settings.pin_tft_d6 = -1;
  tft_settings.pin_tft_d7 = -1;
#endif

#if defined (TFT_BL)
  tft_settings.pin_tft_led = TFT_BL;
#endif

#if defined (TFT_BACKLIGHT_ON)
  tft_settings.pin_tft_led_on = TFT_BACKLIGHT_ON;
#endif

#if defined (TOUCH_CS)
  tft_settings.pin_tch_cs   = TOUCH_CS;
  tft_settings.tch_spi_freq = SPI_TOUCH_FREQUENCY/100000;
#else
  tft_settings.pin_tch_cs   = -1;
  tft_settings.tch_spi_freq = 0;
#endif
}


////////////////////////////////////////////////////////////////////////////////////////
#include "Extensions/Button.cpp"

#include "Extensions/Sprite.cpp"
#include "Extensions/Sprite_CHAR.cpp"

#ifdef SMOOTH_FONT
  #include "Extensions/Sprite_Smooth_font.cpp"
#endif

#ifdef SMOOTH_FONT
  #include "Extensions/Smooth_font_show.cpp"
#endif

#ifdef AA_GRAPHICS
  #include "Extensions/AA_graphics.cpp"  // Loaded if SMOOTH_FONT is defined by user
#endif
////////////////////////////////////////////////////////////////////////////////////////


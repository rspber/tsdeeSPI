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

void TFT_eSPI::setRotationSizes(uint8_t r)
{
  if (r & 1) {
    _clip.width  = _init_height;
    _clip.height = _init_width;
  }
  else {
    _clip.width  = _init_width;
    _clip.height = _init_height;
  }
}


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
bool TFT_eSPI::checkViewport(int32_t x, int32_t y, int32_t w, int32_t h)
{
  block_t z;
  if (!_clip.check_block(z, x, y, w, h)) return false;

  return true;
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
TFT_eSPI::TFT_eSPI(int16_t w, int16_t h) : TFT_CHAR()
{
  _init_width  = _clip.width  = w; // Set by specific xxxxx_Defines.h file or by users sketch
  _init_height = _clip.height = h; // Set by specific xxxxx_Defines.h file or by users sketch

  // Reset the viewport to the whole screen
  resetViewport();

  _cursor.y  = _cursor.x  = _last_cursor_x = _bg_cursor_x = 0;
  _font.font  = 1;
  _font.size  = 1;
  _textcolor   = WHITE; // White
  _textbgcolor = BLACK; // Black
  _padX        = 0;                  // No padding

  _textdatum = TL_DATUM; // Top Left text alignment is default

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
void TFT_eSPI::pushImage16(int32_t x, int32_t y, int32_t w, int32_t h, const uint8_t *data, bool bpp8, rgb_t *cmap)
{
  pushImage16(_clip, x, y, w, h, data, bpp8, cmap);
}


/***************************************************************************************
** Function name:           pushImage
** Description:             plot 8-bit or 4-bit or 1 bit image or sprite using a line buffer
***************************************************************************************/
void TFT_eSPI::pushImage16(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t *data, bool bpp8, rgb_t *cmap)
{
  pushImage16(_clip, x, y, w, h, data, bpp8, cmap);
}


/***************************************************************************************
** Function name:           pushImage
** Description:             plot 8 or 4 or 1 bit image or sprite with a transparent colour
***************************************************************************************/
void TFT_eSPI::pushImage16(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t *data, uint8_t transp, bool bpp8, rgb_t *cmap)
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
  fillRect(0, 0, _clip.width, _clip.height, color);
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
** Function name:           setCursor
** Description:             Set the text cursor x,y position
***************************************************************************************/
void TFT_eSPI::setCursor(int16_t x, int16_t y)
{
  _cursor.x = x;
  _cursor.y = y;
}


/***************************************************************************************
** Function name:           setCursor
** Description:             Set the text cursor x,y position and font
***************************************************************************************/
void TFT_eSPI::setCursor(int16_t x, int16_t y, uint8_t textfont)
{
  setTextFont(textfont);
  _cursor.x = x;
  _cursor.y = y;
}


/***************************************************************************************
** Function name:           getCursorX
** Description:             Get the text cursor x position
***************************************************************************************/
int16_t TFT_eSPI::getCursorX(void)
{
  return _cursor.x;
}

/***************************************************************************************
** Function name:           getCursorY
** Description:             Get the text cursor y position
***************************************************************************************/
int16_t TFT_eSPI::getCursorY(void)
{
  return _cursor.y;
}


/***************************************************************************************
** Function name:           setTextSize
** Description:             Set the text size multiplier
***************************************************************************************/
void TFT_eSPI::setTextSize(uint8_t s)
{
  if (s>7) s = 7; // Limit the maximum size multiplier so byte variables can be used for rendering
  _font.size = (s > 0) ? s : 1; // Don't allow font size 0
}


/***************************************************************************************
** Function name:           setTextColor
** Description:             Set the font foreground colour (background is transparent)
***************************************************************************************/
void TFT_eSPI::setTextColor(uint16_t c)
{
  // For 'transparent' background, we'll set the bg
  // to the same as fg instead of using a flag
  _textcolor = _textbgcolor = c;
}


/***************************************************************************************
** Function name:           setTextColor
** Description:             Set the font foreground and background colour
***************************************************************************************/
// Smooth fonts use the background colour for anti-aliasing and by default the
// background is not filled. If bgfill = true, then a smooth font background fill will
// be used.
void TFT_eSPI::setTextColor(uint16_t c, uint16_t b, bool bgfill)
{
  _textcolor   = c;
  _textbgcolor = b;
  _fillbg     = bgfill;
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
** Function name:           setTextWrap
** Description:             Define if text should wrap at end of line
***************************************************************************************/
void TFT_eSPI::setTextWrap(bool wrapX, bool wrapY)
{
  _textwrapX = wrapX;
  _textwrapY = wrapY;
}


/***************************************************************************************
** Function name:           setTextDatum
** Description:             Set the text position reference datum
***************************************************************************************/
void TFT_eSPI::setTextDatum(uint8_t d)
{
  _textdatum = d;
}


/***************************************************************************************
** Function name:           setTextPadding
** Description:             Define padding width (aids erasing old text and numbers)
***************************************************************************************/
void TFT_eSPI::setTextPadding(uint16_t x_width)
{
  _padX = x_width;
}

/***************************************************************************************
** Function name:           setTextPadding
** Description:             Define padding width (aids erasing old text and numbers)
***************************************************************************************/
uint16_t TFT_eSPI::getTextPadding(void)
{
  return _padX;
}

/***************************************************************************************
** Function name:           getTextDatum
** Description:             Return the text datum value (as used by setTextDatum())
***************************************************************************************/
uint8_t TFT_eSPI::getTextDatum(void)
{
  return _textdatum;
}


/***************************************************************************************
** Function name:           width
** Description:             Return the pixel width of display (per current rotation)
***************************************************************************************/
// Return the size of the display (per current rotation)
int16_t TFT_eSPI::width(void)
{
  if (_vpDatum) return _xWidth;
  return _clip.width;
}


/***************************************************************************************
** Function name:           height
** Description:             Return the pixel height of display (per current rotation)
***************************************************************************************/
int16_t TFT_eSPI::height(void)
{
  if (_vpDatum) return _yHeight;
  return _clip.height;
}


/***************************************************************************************
** Function name:           textWidth
** Description:             Return the width in pixels of a string in a given font
***************************************************************************************/
int16_t TFT_eSPI::textWidth(const String& string)
{
  int16_t len = string.length() + 2;
  char buffer[len];
  string.toCharArray(buffer, len);
  return textWidth(_font, buffer);
}

int16_t TFT_eSPI::textWidth(const String& string, uint8_t textfont)
{
  int16_t len = string.length() + 2;
  char buffer[len];
  string.toCharArray(buffer, len);
  chr_font_t tmp = _font;
  tmp.font = textfont;
  return textWidth(tmp, buffer);
}

int16_t TFT_eSPI::textWidth(const char *string)
{
  return textWidth(_font, string);
}


/***************************************************************************************
** Function name:           fontHeight
** Description:             return the height of a font (yAdvance for free fonts)
***************************************************************************************/
int16_t TFT_eSPI::fontHeight(uint8_t textfont)
{
  if (textfont > 8) return 0;

  chr_font_t tmp = _font;
  tmp.font = textfont;
  return fontHeight(tmp);
}

int16_t TFT_eSPI::fontHeight(void)
{
  return fontHeight(_font);
}

/***************************************************************************************
** Function name:           drawChar
** Description:             draw a single character in the GLCD or GFXFF font
***************************************************************************************/
void TFT_eSPI::drawChar(int32_t x, int32_t y, uint16_t c, rgb_t color, rgb_t bg, uint8_t textsize)
{
  cursor_t cursor;
  cursor.x = x;
  cursor.y = y;
  chr_font_t tmp = _font;
  tmp.size = textsize;
  drawChar(_clip, cursor, tmp, c, color, bg);
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
** Function name:           write
** Description:             draw characters piped through serial stream
***************************************************************************************/
/* // Not all processors support buffered write
#ifndef ARDUINO_ARCH_ESP8266 // Avoid ESP8266 board package bug
size_t TFT_eSPI::write(const uint8_t *buf, size_t len)
{
  inTransaction = true;

  uint8_t *lbuf = (uint8_t *)buf;
  while(*lbuf !=0 && len--) write(*lbuf++);

  inTransaction = lockTransaction;
  end_tft_write();
  return 1;
}
#endif
*/
/***************************************************************************************
** Function name:           write
** Description:             draw characters piped through serial stream
***************************************************************************************/
size_t TFT_eSPI::write(uint8_t utf8)
{
  return write(_clip, _cursor, _font, utf8, _textcolor, _textbgcolor);
}


/***************************************************************************************
** Function name:           drawChar
** Description:             draw a Unicode glyph onto the screen
***************************************************************************************/
  // TODO: Rationalise with TFT_eSprite
  // Any UTF-8 decoding must be done before calling drawChar()
int16_t TFT_eSPI::drawChar(uint16_t uniCode, int32_t x, int32_t y)
{
  cursor_t cursor;
  cursor.x = x;
  cursor.y = y;
  return drawChar(_clip, cursor, _font, uniCode, _textcolor, _textbgcolor);
}

  // Any UTF-8 decoding must be done before calling drawChar()
int16_t TFT_eSPI::drawChar(uint16_t uniCode, int32_t x, int32_t y, uint8_t textfont)
{
  cursor_t cursor;
  cursor.x = x;
  cursor.y = y;
  chr_font_t tmp = _font;
  tmp.font = textfont;
  return drawChar(_clip, cursor, tmp, uniCode, _textcolor, _textbgcolor);
}


/***************************************************************************************
** Function name:           drawString (with or without user defined font)
** Description :            draw string with padding if it is defined
***************************************************************************************/
// Without font number, uses font set by setTextFont()
int16_t TFT_eSPI::drawString(const String& string, int32_t poX, int32_t poY)
{
  int16_t len = string.length() + 2;
  char buffer[len];
  string.toCharArray(buffer, len);
  return drawString(buffer, poX, poY, _font.font);
}
// With font number
int16_t TFT_eSPI::drawString(const String& string, int32_t poX, int32_t poY, uint8_t textfont)
{
  int16_t len = string.length() + 2;
  char buffer[len];
  string.toCharArray(buffer, len);
  return drawString(buffer, poX, poY, textfont);
}

// Without font number, uses font set by setTextFont()
int16_t TFT_eSPI::drawString(const char *string, int32_t poX, int32_t poY)
{
  return drawString(string, poX, poY, _font.font);
}

// With font number. Note: font number is over-ridden if a smooth font is loaded
int16_t TFT_eSPI::drawString(const char *string, int32_t poX, int32_t poY, uint8_t textfont)
{
  chr_font_t font = _font;
  font.font = textfont;

  int16_t sumX = 0;
  uint8_t padding = 1, baseline = 0;
  uint16_t cwidth = textWidth(font, string); // Find the pixel width of the string in the font
  uint16_t cheight = 8 * font.size;

#ifdef LOAD_GFXFF
  #ifdef SMOOTH_FONT
    bool freeFont = (font.font == 1 && font.gfxFont && !fontLoaded);
  #else
    bool freeFont = (font.font == 1 && font.gfxFont);
  #endif

  if (freeFont) {
    cheight = _glyph_ab * font.size;
    poY += cheight; // Adjust for baseline datum of free fonts
    baseline = cheight;
    padding =101; // Different padding method used for Free Fonts

    // We need to make an adjustment for the bottom of the string (eg 'y' character)
    if ((_textdatum == BL_DATUM) || (_textdatum == BC_DATUM) || (_textdatum == BR_DATUM)) {
      cheight += _glyph_bb * font.size;
    }
  }
#endif


  // If it is not font 1 (GLCD or free font) get the baseline and pixel height of the font
#ifdef SMOOTH_FONT
  if(fontLoaded) {
    baseline = gFont.maxAscent;
    cheight  = fontHeight(font);
  }
  else
#endif
  if (font.font!=1) {
    baseline = pgm_read_byte( &fontdata[font.font].baseline ) * font.size;
    cheight = fontHeight(font);
  }

  if (_textdatum || _padX) {

    switch(_textdatum) {
      case TC_DATUM:
        poX -= cwidth/2;
        padding += 1;
        break;
      case TR_DATUM:
        poX -= cwidth;
        padding += 2;
        break;
      case ML_DATUM:
        poY -= cheight/2;
        //padding += 0;
        break;
      case MC_DATUM:
        poX -= cwidth/2;
        poY -= cheight/2;
        padding += 1;
        break;
      case MR_DATUM:
        poX -= cwidth;
        poY -= cheight/2;
        padding += 2;
        break;
      case BL_DATUM:
        poY -= cheight;
        //padding += 0;
        break;
      case BC_DATUM:
        poX -= cwidth/2;
        poY -= cheight;
        padding += 1;
        break;
      case BR_DATUM:
        poX -= cwidth;
        poY -= cheight;
        padding += 2;
        break;
      case L_BASELINE:
        poY -= baseline;
        //padding += 0;
        break;
      case C_BASELINE:
        poX -= cwidth/2;
        poY -= baseline;
        padding += 1;
        break;
      case R_BASELINE:
        poX -= cwidth;
        poY -= baseline;
        padding += 2;
        break;
    }
  }


  int8_t xo = 0;
#ifdef LOAD_GFXFF
  if (freeFont && (_textcolor!=_textbgcolor)) {
      cheight = (_glyph_ab + _glyph_bb) * font.size;
      // Get the offset for the first character only to allow for negative offsets
      uint16_t c2 = 0;
      uint16_t len = strlen(string);
      uint16_t n = 0;

      while (n < len && c2 == 0) c2 = decodeUTF8((uint8_t*)string, &n, len - n);

      if((c2 >= pgm_read_word(&font.gfxFont->first)) && (c2 <= pgm_read_word(&font.gfxFont->last) )) {
        c2 -= pgm_read_word(&font.gfxFont->first);
        GFXglyph *glyph = &(((GFXglyph *)pgm_read_dword(&font.gfxFont->glyph))[c2]);
        xo = pgm_read_byte(&glyph->xOffset) * font.size;
        // Adjust for negative xOffset
        if (xo > 0) xo = 0;
        else cwidth -= xo;
        // Add 1 pixel of padding all round
        //cheight +=2;
        //fillRect(poX+xo-1, poY - 1 - glyph_ab * font.size, cwidth+2, cheight, textbgcolor);
        fillRect(_clip, poX+xo, poY - _glyph_ab * font.size, cwidth, cheight, _textbgcolor);
      }
      padding -=100;
    }
#endif

  uint16_t len = strlen(string);
  uint16_t n = 0;

#ifdef SMOOTH_FONT
  if(fontLoaded) {
    setCursor(poX, poY);

    bool fillbg = _fillbg;
    // If padding is requested then fill the text background
    if (_padX && !_fillbg) _fillbg = true;

    while (n < len) {
      uint16_t uniCode = decodeUTF8((uint8_t*)string, &n, len - n);
      drawGlyph(_clip, _cursor, uniCode, _textcolor, _textbgcolor);
    }
    _fillbg = fillbg; // restore state
    sumX += cwidth;
    //fontFile.close();
  }
  else
#endif
  {
    while (n < len) {
      uint16_t uniCode = decodeUTF8((uint8_t*)string, &n, len - n);
      cursor_t cursor;
      cursor.x = poX+sumX;
      cursor.y = poY;
      sumX += drawChar(_clip, cursor, font, uniCode, _textcolor, _textbgcolor);
    }
  }

//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv DEBUG vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Switch on debugging for the padding areas
//#define PADDING_DEBUG

#ifndef PADDING_DEBUG
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ DEBUG ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  if((_padX>cwidth) && (_textcolor!=_textbgcolor)) {
    int16_t padXc = poX+cwidth+xo;
#ifdef LOAD_GFXFF
    if (freeFont) {
      poX +=xo; // Adjust for negative offset start character
      poY -= _glyph_ab * font.size;
      sumX += poX;
    }
#endif
    switch(padding) {
      case 1:
        fillRect(_clip, padXc,poY,_padX-cwidth,cheight, _textbgcolor);
        break;
      case 2:
        fillRect(_clip, padXc,poY,(_padX-cwidth)>>1,cheight, _textbgcolor);
        padXc = poX - ((_padX-cwidth)>>1);
        fillRect(_clip, padXc,poY,(_padX-cwidth)>>1,cheight, _textbgcolor);
        break;
      case 3:
        if (padXc>_padX) padXc = _padX;
        fillRect(_clip, poX + cwidth - padXc,poY,padXc-cwidth,cheight, _textbgcolor);
        break;
    }
  }


#else

//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv DEBUG vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// This is debug code to show text (green box) and blanked (white box) areas
// It shows that the padding areas are being correctly sized and positioned

  if((_padX>sumX) && (_textcolor!=_textbgcolor)) {
    int16_t padXc = poX+sumX; // Maximum left side padding
#ifdef LOAD_GFXFF
    if ((font.font == 1) && (font.gfxFont)) poY -= _glyph_ab;
#endif
    drawRect(poX,poY,sumX,cheight, TFT_GREEN);
    switch(padding) {
      case 1:
        drawRect(_clip, padXc,poY,_padX-sumX,cheight, TFT_WHITE);
        break;
      case 2:
        drawRect(_clip, padXc,poY,(_padX-sumX)>>1, cheight, TFT_WHITE);
        padXc = (_padX-sumX)>>1;
        drawRect(_clip, poX - padXc,poY,(_padX-sumX)>>1,cheight, TFT_WHITE);
        break;
      case 3:
        if (padXc>_padX) padXc = _padX;
        drawRect(_clip, poX + sumX - padXc,poY,padXc-sumX,cheight, TFT_WHITE);
        break;
    }
  }
#endif
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ DEBUG ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

return sumX;
}


/***************************************************************************************
** Function name:           drawCentreString (deprecated, use setTextDatum())
** Descriptions:            draw string centred on dX
***************************************************************************************/
int16_t TFT_eSPI::drawCentreString(const String& string, int32_t dX, int32_t poY, uint8_t textfont)
{
  int16_t len = string.length() + 2;
  char buffer[len];
  string.toCharArray(buffer, len);
  return drawCentreString(buffer, dX, poY, textfont);
}

int16_t TFT_eSPI::drawCentreString(const char *string, int32_t dX, int32_t poY, uint8_t textfont)
{
  uint8_t tempdatum = _textdatum;
  int32_t sumX = 0;
  _textdatum = TC_DATUM;
  sumX = drawString(string, dX, poY, textfont);
  _textdatum = tempdatum;
  return sumX;
}


/***************************************************************************************
** Function name:           drawRightString (deprecated, use setTextDatum())
** Descriptions:            draw string right justified to dX
***************************************************************************************/
int16_t TFT_eSPI::drawRightString(const String& string, int32_t dX, int32_t poY, uint8_t textfont)
{
  int16_t len = string.length() + 2;
  char buffer[len];
  string.toCharArray(buffer, len);
  return drawRightString(buffer, dX, poY, textfont);
}

int16_t TFT_eSPI::drawRightString(const char *string, int32_t dX, int32_t poY, uint8_t textfont)
{
  uint8_t tempdatum = _textdatum;
  int16_t sumX = 0;
  _textdatum = TR_DATUM;
  sumX = drawString(string, dX, poY, textfont);
  _textdatum = tempdatum;
  return sumX;
}


/***************************************************************************************
** Function name:           drawNumber
** Description:             draw a long integer
***************************************************************************************/
int16_t TFT_eSPI::drawNumber(long long_num, int32_t poX, int32_t poY)
{
  _isDigits = true; // Eliminate jiggle in monospaced fonts
  char str[12];
  ltoa(long_num, str, 10);
  return drawString(str, poX, poY, _font.font);
}

int16_t TFT_eSPI::drawNumber(long long_num, int32_t poX, int32_t poY, uint8_t textfont)
{
  _isDigits = true; // Eliminate jiggle in monospaced fonts
  char str[12];
  ltoa(long_num, str, 10);
  return drawString(str, poX, poY, textfont);
}


/***************************************************************************************
** Function name:           drawFloat
** Descriptions:            drawFloat, prints 7 non zero digits maximum
***************************************************************************************/
// Assemble and print a string, this permits alignment relative to a datum
// looks complicated but much more compact and actually faster than using print class
int16_t TFT_eSPI::drawFloat(float floatNumber, uint8_t dp, int32_t poX, int32_t poY)
{
  return drawFloat(floatNumber, dp, poX, poY, _font.font);
}

int16_t TFT_eSPI::drawFloat(float floatNumber, uint8_t dp, int32_t poX, int32_t poY, uint8_t textfont)
{
  _isDigits = true;
  char str[14];               // Array to contain decimal string
  uint8_t ptr = 0;            // Initialise pointer for array
  int8_t  digits = 1;         // Count the digits to avoid array overflow
  float rounding = 0.5;       // Round up down delta
  bool negative = false;

  if (dp > 7) dp = 7; // Limit the size of decimal portion

  // Adjust the rounding value
  for (uint8_t i = 0; i < dp; ++i) rounding /= 10.0;

  if (floatNumber < -rounding) {   // add sign, avoid adding - sign to 0.0!
    str[ptr++] = '-'; // Negative number
    str[ptr] = 0; // Put a null in the array as a precaution
    digits = 0;   // Set digits to 0 to compensate so pointer value can be used later
    floatNumber = -floatNumber; // Make positive
    negative = true;
  }

  floatNumber += rounding; // Round up or down

  if (dp == 0) {
    if (negative) floatNumber = -floatNumber;
    return drawNumber((long)floatNumber, poX, poY, textfont);
  }

  // For error put ... in string and return (all TFT_eSPI library fonts contain . character)
  if (floatNumber >= 2147483647) {
    strcpy(str, "...");
    return drawString(str, poX, poY, textfont);
  }
  // No chance of overflow from here on

  // Get integer part
  uint32_t temp = (uint32_t)floatNumber;

  // Put integer part into array
  ltoa(temp, str + ptr, 10);

  // Find out where the null is to get the digit count loaded
  while ((uint8_t)str[ptr] != 0) ptr++; // Move the pointer along
  digits += ptr;                  // Count the digits

  str[ptr++] = '.'; // Add decimal point
  str[ptr] = '0';   // Add a dummy zero
  str[ptr + 1] = 0; // Add a null but don't increment pointer so it can be overwritten

  // Get the decimal portion
  floatNumber = floatNumber - temp;

  // Get decimal digits one by one and put in array
  // Limit digit count so we don't get a false sense of resolution
  uint8_t i = 0;
  while ((i < dp) && (digits < 9)) { // while (i < dp) for no limit but array size must be increased
    i++;
    floatNumber *= 10;       // for the next decimal
    temp = floatNumber;      // get the decimal
    ltoa(temp, str + ptr, 10);
    ptr++; digits++;         // Increment pointer and digits count
    floatNumber -= temp;     // Remove that digit
  }

  // Finally we can plot the string and return pixel length
  return drawString(str, poX, poY, textfont);
}


/***************************************************************************************
** Function name:           setFreeFont
** Descriptions:            Sets the GFX free font to use
***************************************************************************************/

#ifdef LOAD_GFXFF

void TFT_eSPI::setFreeFont(const GFXfont *f)
{
  if (f == nullptr) { // Fix issue #400 (ESP32 crash)
    setTextFont(1); // Use GLCD font
    return;
  }

  _font.font = 1;
  _font.gfxFont = (GFXfont *)f;

  _glyph_ab = 0;
  _glyph_bb = 0;
  uint16_t numChars = pgm_read_word(&_font.gfxFont->last) - pgm_read_word(&_font.gfxFont->first);

  // Find the biggest above and below baseline offsets
  for (uint16_t c = 0; c < numChars; c++) {
    GFXglyph *glyph1  = &(((GFXglyph *)pgm_read_dword(&_font.gfxFont->glyph))[c]);
    int8_t ab = -pgm_read_byte(&glyph1->yOffset);
    if (ab > _glyph_ab) _glyph_ab = ab;
    int8_t bb = pgm_read_byte(&glyph1->height) - ab;
    if (bb > _glyph_bb) _glyph_bb = bb;
  }
}


/***************************************************************************************
** Function name:           setTextFont
** Description:             Set the font for the print stream
***************************************************************************************/
void TFT_eSPI::setTextFont(uint8_t f)
{
  _font.font = (f > 0) ? f : 1; // Don't allow font 0
  _font.gfxFont = NULL;
}

#else


/***************************************************************************************
** Function name:           setFreeFont
** Descriptions:            Sets the GFX free font to use
***************************************************************************************/

// Alternative to setTextFont() so we don't need two different named functions
void TFT_eSPI::setFreeFont(uint8_t textfont)
{
  setTextFont(textfont);
}


/***************************************************************************************
** Function name:           setTextFont
** Description:             Set the font for the print stream
***************************************************************************************/
void TFT_eSPI::setTextFont(uint8_t f)
{
  _font.font = (f > 0) ? f : 1; // Don't allow font 0
}
#endif


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

#ifdef SMOOTH_FONT
  #include "Extensions/Smooth_font_show.cpp"
#endif

#ifdef AA_GRAPHICS
  #include "Extensions/AA_graphics.cpp"  // Loaded if SMOOTH_FONT is defined by user
#endif
////////////////////////////////////////////////////////////////////////////////////////


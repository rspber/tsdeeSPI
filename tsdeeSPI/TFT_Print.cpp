/*
  Print functions

  Copyright (c) 2024, rspber (https://github.com/rspber)

  Based on: TFT_Print

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

#include "TFT_Print.h"

/***************************************************************************************
** Function name:           TFT_Print
** Description:             Constructor , we must use hardware SPI pins
***************************************************************************************/
TFT_Print::TFT_Print() : TFT_CHAR()
{
  _cursor.y  = _cursor.x = 0;
  _font.font  = 1;
  _font.size  = 1;
  _textcolor   = WHITE; // White
  _textbgcolor = BLACK; // Black
  _padX        = 0;                  // No padding

  _textdatum = TL_DATUM; // Top Left text alignment is default
}

/***************************************************************************************
** Function name:           setCursor
** Description:             Set the text cursor x,y position
***************************************************************************************/
void TFT_Print::setCursor(int16_t x, int16_t y)
{
  _cursor.x = x;
  _cursor.y = y;
}


/***************************************************************************************
** Function name:           setCursor
** Description:             Set the text cursor x,y position and font
***************************************************************************************/
void TFT_Print::setCursor(int16_t x, int16_t y, uint8_t textfont)
{
  setTextFont(textfont);
  _cursor.x = x;
  _cursor.y = y;
}


/***************************************************************************************
** Function name:           getCursorX
** Description:             Get the text cursor x position
***************************************************************************************/
int16_t TFT_Print::getCursorX(void)
{
  return _cursor.x;
}

/***************************************************************************************
** Function name:           getCursorY
** Description:             Get the text cursor y position
***************************************************************************************/
int16_t TFT_Print::getCursorY(void)
{
  return _cursor.y;
}


/***************************************************************************************
** Function name:           setTextSize
** Description:             Set the text size multiplier
***************************************************************************************/
void TFT_Print::setTextSize(uint8_t s)
{
  if (s>7) s = 7; // Limit the maximum size multiplier so byte variables can be used for rendering
  _font.size = (s > 0) ? s : 1; // Don't allow font size 0
}


/***************************************************************************************
** Function name:           setTextColor
** Description:             Set the font foreground colour (background is transparent)
***************************************************************************************/
void TFT_Print::setTextColor(uint16_t c)
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
void TFT_Print::setTextColor(uint16_t c, uint16_t b, bool bgfill)
{
  _textcolor   = c;
  _textbgcolor = b;
  _fillbg     = bgfill;
}


/***************************************************************************************
** Function name:           setTextWrap
** Description:             Define if text should wrap at end of line
***************************************************************************************/
void TFT_Print::setTextWrap(bool wrapX, bool wrapY)
{
  _textwrapX = wrapX;
  _textwrapY = wrapY;
}


/***************************************************************************************
** Function name:           setTextDatum
** Description:             Set the text position reference datum
***************************************************************************************/
void TFT_Print::setTextDatum(uint8_t d)
{
  _textdatum = d;
}


/***************************************************************************************
** Function name:           setTextPadding
** Description:             Define padding width (aids erasing old text and numbers)
***************************************************************************************/
void TFT_Print::setTextPadding(uint16_t x_width)
{
  _padX = x_width;
}

/***************************************************************************************
** Function name:           setTextPadding
** Description:             Define padding width (aids erasing old text and numbers)
***************************************************************************************/
uint16_t TFT_Print::getTextPadding(void)
{
  return _padX;
}

/***************************************************************************************
** Function name:           getTextDatum
** Description:             Return the text datum value (as used by setTextDatum())
***************************************************************************************/
uint8_t TFT_Print::getTextDatum(void)
{
  return _textdatum;
}


/***************************************************************************************
** Function name:           textWidth
** Description:             Return the width in pixels of a string in a given font
***************************************************************************************/
int16_t TFT_Print::textWidth(const String& string)
{
  int16_t len = string.length() + 2;
  char buffer[len];
  string.toCharArray(buffer, len);
  return textWidth(_font, buffer);
}

int16_t TFT_Print::textWidth(const String& string, uint8_t textfont)
{
  int16_t len = string.length() + 2;
  char buffer[len];
  string.toCharArray(buffer, len);
  chr_font_t tmp = _font;
  tmp.font = textfont;
  return textWidth(tmp, buffer);
}

int16_t TFT_Print::textWidth(const char *string)
{
  return textWidth(_font, string);
}


/***************************************************************************************
** Function name:           fontHeight
** Description:             return the height of a font (yAdvance for free fonts)
***************************************************************************************/
int16_t TFT_Print::fontHeight(uint8_t textfont)
{
  if (textfont > 8) return 0;

  chr_font_t tmp = _font;
  tmp.font = textfont;
  return fontHeight(tmp);
}

int16_t TFT_Print::fontHeight(void)
{
  return fontHeight(_font);
}


/***************************************************************************************
** Function name:           write
** Description:             draw characters piped through serial stream
***************************************************************************************/
/* // Not all processors support buffered write
#ifndef ARDUINO_ARCH_ESP8266 // Avoid ESP8266 board package bug
size_t TFT_Print::write(const uint8_t *buf, size_t len)
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
size_t TFT_Print::write(uint8_t utf8)
{
  return write(_clip, _cursor, _font, utf8, _textcolor, _textbgcolor);
}


/***************************************************************************************
** Function name:           drawChar
** Description:             draw a single character in the GLCD or GFXFF font
***************************************************************************************/
void TFT_Print::drawChar(int32_t x, int32_t y, uint16_t c, rgb_t color, rgb_t bg, uint8_t textsize)
{
  cursor_t cursor;
  cursor.x = x;
  cursor.y = y;
  chr_font_t tmp = _font;
  tmp.size = textsize;
  drawChar(_clip, cursor, tmp, c, color, bg);
}


/***************************************************************************************
** Function name:           drawChar
** Description:             draw a Unicode glyph onto the screen
***************************************************************************************/
  // TODO: Rationalise with TFT_eSprite
  // Any UTF-8 decoding must be done before calling drawChar()
int16_t TFT_Print::drawChar(uint16_t uniCode, int32_t x, int32_t y)
{
  cursor_t cursor;
  cursor.x = x;
  cursor.y = y;
  return drawChar(_clip, cursor, _font, uniCode, _textcolor, _textbgcolor);
}

  // Any UTF-8 decoding must be done before calling drawChar()
int16_t TFT_Print::drawChar(uint16_t uniCode, int32_t x, int32_t y, uint8_t textfont)
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
int16_t TFT_Print::drawString(const String& string, int32_t poX, int32_t poY)
{
  int16_t len = string.length() + 2;
  char buffer[len];
  string.toCharArray(buffer, len);
  return drawString(buffer, poX, poY, _font.font);
}
// With font number
int16_t TFT_Print::drawString(const String& string, int32_t poX, int32_t poY, uint8_t textfont)
{
  int16_t len = string.length() + 2;
  char buffer[len];
  string.toCharArray(buffer, len);
  return drawString(buffer, poX, poY, textfont);
}

// Without font number, uses font set by setTextFont()
int16_t TFT_Print::drawString(const char *string, int32_t poX, int32_t poY)
{
  return drawString(string, poX, poY, _font.font);
}

// With font number. Note: font number is over-ridden if a smooth font is loaded
int16_t TFT_Print::drawString(const char *string, int32_t poX, int32_t poY, uint8_t textfont)
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
int16_t TFT_Print::drawCentreString(const String& string, int32_t dX, int32_t poY, uint8_t textfont)
{
  int16_t len = string.length() + 2;
  char buffer[len];
  string.toCharArray(buffer, len);
  return drawCentreString(buffer, dX, poY, textfont);
}

int16_t TFT_Print::drawCentreString(const char *string, int32_t dX, int32_t poY, uint8_t textfont)
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
int16_t TFT_Print::drawRightString(const String& string, int32_t dX, int32_t poY, uint8_t textfont)
{
  int16_t len = string.length() + 2;
  char buffer[len];
  string.toCharArray(buffer, len);
  return drawRightString(buffer, dX, poY, textfont);
}

int16_t TFT_Print::drawRightString(const char *string, int32_t dX, int32_t poY, uint8_t textfont)
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
int16_t TFT_Print::drawNumber(long long_num, int32_t poX, int32_t poY)
{
  _isDigits = true; // Eliminate jiggle in monospaced fonts
  char str[12];
  ltoa(long_num, str, 10);
  return drawString(str, poX, poY, _font.font);
}

int16_t TFT_Print::drawNumber(long long_num, int32_t poX, int32_t poY, uint8_t textfont)
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
int16_t TFT_Print::drawFloat(float floatNumber, uint8_t dp, int32_t poX, int32_t poY)
{
  return drawFloat(floatNumber, dp, poX, poY, _font.font);
}

int16_t TFT_Print::drawFloat(float floatNumber, uint8_t dp, int32_t poX, int32_t poY, uint8_t textfont)
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

  // For error put ... in string and return (all TFT_Print library fonts contain . character)
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

void TFT_Print::setFreeFont(const GFXfont *f)
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
void TFT_Print::setTextFont(uint8_t f)
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
void TFT_Print::setFreeFont(uint8_t textfont)
{
  setTextFont(textfont);
}


/***************************************************************************************
** Function name:           setTextFont
** Description:             Set the font for the print stream
***************************************************************************************/
void TFT_Print::setTextFont(uint8_t f)
{
  _font.font = (f > 0) ? f : 1; // Don't allow font 0
}
#endif

/*
  Print functions

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

// Stop fonts etc. being loaded multiple times
#pragma once

#define TFT_ESPI_VERSION "2.5.43"

#include "TFT_CHAR.h"

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
//These enumerate the text plotting alignment (reference datum point)
#define TL_DATUM 0 // Top left (default)
#define TC_DATUM 1 // Top centre
#define TR_DATUM 2 // Top right
#define ML_DATUM 3 // Middle left
#define CL_DATUM 3 // Centre left, same as above
#define MC_DATUM 4 // Middle centre
#define CC_DATUM 4 // Centre centre, same as above
#define MR_DATUM 5 // Middle right
#define CR_DATUM 5 // Centre right, same as above
#define BL_DATUM 6 // Bottom left
#define BC_DATUM 7 // Bottom centre
#define BR_DATUM 8 // Bottom right
#define L_BASELINE  9 // Left character baseline (Line the 'A' character would sit on)
#define C_BASELINE 10 // Centre character baseline
#define R_BASELINE 11 // Right character baseline

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
class TFT_Print : public TFT_CHAR {

 //--------------------------------------- public ------------------------------------//
 public:

  TFT_Print();

  void drawChar(int32_t x, int32_t y, uint16_t c, rgb_t color, rgb_t bg, uint8_t size);

  int16_t  drawChar(uint16_t uniCode, int32_t x, int32_t y, uint8_t font),
           drawChar(uint16_t uniCode, int32_t x, int32_t y);

  // Text rendering - value returned is the pixel width of the rendered text
  int16_t  drawNumber(long intNumber, int32_t x, int32_t y, uint8_t font), // Draw integer using specified font number
           drawNumber(long intNumber, int32_t x, int32_t y),               // Draw integer using current font

           // Decimal is the number of decimal places to render
           // Use with setTextDatum() to position values on TFT, and setTextPadding() to blank old displayed values
           drawFloat(float floatNumber, uint8_t decimal, int32_t x, int32_t y, uint8_t font), // Draw float using specified font number
           drawFloat(float floatNumber, uint8_t decimal, int32_t x, int32_t y),               // Draw float using current font

           // Handle char arrays
           // Use with setTextDatum() to position string on TFT, and setTextPadding() to blank old displayed strings
           drawString(const char *string, int32_t x, int32_t y, uint8_t font),  // Draw string using specified font number
           drawString(const char *string, int32_t x, int32_t y),                // Draw string using current font
           drawString(const String& string, int32_t x, int32_t y, uint8_t font),// Draw string using specified font number
           drawString(const String& string, int32_t x, int32_t y),              // Draw string using current font

           drawCentreString(const char *string, int32_t x, int32_t y, uint8_t font),  // Deprecated, use setTextDatum() and drawString()
           drawRightString(const char *string, int32_t x, int32_t y, uint8_t font),   // Deprecated, use setTextDatum() and drawString()
           drawCentreString(const String& string, int32_t x, int32_t y, uint8_t font),// Deprecated, use setTextDatum() and drawString()
           drawRightString(const String& string, int32_t x, int32_t y, uint8_t font); // Deprecated, use setTextDatum() and drawString()


  // Text rendering and font handling support functions
  void     setCursor(int16_t x, int16_t y),                 // Set cursor for tft.print()
           setCursor(int16_t x, int16_t y, uint8_t font);   // Set cursor and font number for tft.print()

  int16_t  getCursorX(void),                                // Read current cursor x position (moves with tft.print())
           getCursorY(void);                                // Read current cursor y position

  void     setTextColor(uint16_t color),                    // Set character (glyph) color only (background not over-written)
           setTextColor(uint16_t fgcolor, uint16_t bgcolor, bool bgfill = false),  // Set character (glyph) foreground and background colour, optional background fill for smooth fonts
           setTextSize(uint8_t size);                       // Set character size multiplier (this increases pixel size)

  void     setTextWrap(bool wrapX, bool wrapY = false);     // Turn on/off wrapping of text in TFT width and/or height

  void     setTextDatum(uint8_t datum);                     // Set text datum position (default is top left), see Section 5 above
  uint8_t  getTextDatum(void);

  void     setTextPadding(uint16_t x_width);                // Set text padding (background blanking/over-write) width in pixels
  uint16_t getTextPadding(void);                            // Get text padding

#ifdef LOAD_GFXFF
  void     setFreeFont(const GFXfont *f = NULL),            // Select the GFX Free Font
           setTextFont(uint8_t font);                       // Set the font number to use in future
#else
  void     setFreeFont(uint8_t font),                       // Not used, historical fix to prevent an error
           setTextFont(uint8_t font);                       // Set the font number to use in future
#endif
  uint8_t  getTextFont();

  int16_t  textWidth(const char *string, uint8_t font),     // Returns pixel width of string in specified font
           textWidth(const char *string),                   // Returns pixel width of string in current font
           textWidth(const String& string, uint8_t font),   // As above for String types
           textWidth(const String& string),
           fontHeight(uint8_t font),                        // Returns pixel height of specified font
           fontHeight(void);                                // Returns pixel height of current font

           // Support function to UTF8 decode and draw characters piped through print stream
  size_t   write(uint8_t utf8) override;
           // size_t   write(const uint8_t *buf, size_t len);

  using TFT_CHAR::textWidth;
  using TFT_CHAR::fontHeight;
  using TFT_CHAR::write;
  using TFT_CHAR::drawChar;

 protected:
  rgb_t _textcolor, _textbgcolor;         // Text foreground and background colours

  chr_font_t _font;

  uint8_t  _textdatum; // Text reference datum

 protected:
  bool     _vpDatum;

  cursor_t _cursor;
  int32_t  _padX;       // Text cursor x,y and padding setting

};

/*
  Character drawing level

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

#include "TFT_GFX.h"

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
// Use GLCD font in error case where user requests a smooth font file
// that does not exist (this is a temporary fix to stop ESP32 reboot)
#ifdef SMOOTH_FONT
  #ifndef LOAD_GLCD
    #define LOAD_GLCD
  #endif
#endif

// Only load the fonts defined in User_Setup.h (to save space)
// Set flag so RLE rendering code is optionally compiled
#ifdef LOAD_GLCD
  #include <Fonts/glcdfont.c>
#endif

#ifdef LOAD_FONT2
  #include <Fonts/Font16.h>
#endif

#ifdef LOAD_FONT4
  #include <Fonts/Font32rle.h>
  #define LOAD_RLE
#endif

#ifdef LOAD_FONT6
  #include <Fonts/Font64rle.h>
  #ifndef LOAD_RLE
    #define LOAD_RLE
  #endif
#endif

#ifdef LOAD_FONT7
  #include <Fonts/Font7srle.h>
  #ifndef LOAD_RLE
    #define LOAD_RLE
  #endif
#endif

#ifdef LOAD_FONT8
  #include <Fonts/Font72rle.h>
  #ifndef LOAD_RLE
    #define LOAD_RLE
  #endif
#elif defined LOAD_FONT8N // Optional narrower version
  #define LOAD_FONT8
  #include <Fonts/Font72x53rle.h>
  #ifndef LOAD_RLE
    #define LOAD_RLE
  #endif
#endif

#ifdef LOAD_GFXFF
  // We can include all the free fonts and they will only be built into
  // the sketch if they are used
  #include <Fonts/GFXFF/gfxfont.h>
  // Call up any user custom fonts
  #include <User_Setups/User_Custom_Fonts.h>
#endif // #ifdef LOAD_GFXFF

// Create a null default font in case some fonts not used (to prevent crash)
const  uint8_t widtbl_null[1] = {0};
PROGMEM const uint8_t chr_null[1] = {0};
PROGMEM const uint8_t* const chrtbl_null[1] = {chr_null};

// This is a structure to conveniently hold information on the default fonts
// Stores pointer to font character image address table, width table and height
typedef struct {
    const uint8_t *chartbl;
    const uint8_t *widthtbl;
    uint8_t height;
    uint8_t baseline;
    } fontinfo;

// Now fill the structure
const PROGMEM fontinfo fontdata [] = {
  #ifdef LOAD_GLCD
   { (const uint8_t *)font, widtbl_null, 0, 0 },
  #else
   { (const uint8_t *)chrtbl_null, widtbl_null, 0, 0 },
  #endif
   // GLCD font (Font 1) does not have all parameters
   { (const uint8_t *)chrtbl_null, widtbl_null, 8, 7 },

  #ifdef LOAD_FONT2
   { (const uint8_t *)chrtbl_f16, widtbl_f16, chr_hgt_f16, baseline_f16},
  #else
   { (const uint8_t *)chrtbl_null, widtbl_null, 0, 0 },
  #endif

   // Font 3 current unused
   { (const uint8_t *)chrtbl_null, widtbl_null, 0, 0 },

  #ifdef LOAD_FONT4
   { (const uint8_t *)chrtbl_f32, widtbl_f32, chr_hgt_f32, baseline_f32},
  #else
   { (const uint8_t *)chrtbl_null, widtbl_null, 0, 0 },
  #endif

   // Font 5 current unused
   { (const uint8_t *)chrtbl_null, widtbl_null, 0, 0 },

  #ifdef LOAD_FONT6
   { (const uint8_t *)chrtbl_f64, widtbl_f64, chr_hgt_f64, baseline_f64},
  #else
   { (const uint8_t *)chrtbl_null, widtbl_null, 0, 0 },
  #endif

  #ifdef LOAD_FONT7
   { (const uint8_t *)chrtbl_f7s, widtbl_f7s, chr_hgt_f7s, baseline_f7s},
  #else
   { (const uint8_t *)chrtbl_null, widtbl_null, 0, 0 },
  #endif

  #ifdef LOAD_FONT8
   { (const uint8_t *)chrtbl_f72, widtbl_f72, chr_hgt_f72, baseline_f72}
  #else
   { (const uint8_t *)chrtbl_null, widtbl_null, 0, 0 }
  #endif
};

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
// Callback prototype for smooth font pixel colour read
typedef uint16_t (*getColorCallback)(uint16_t x, uint16_t y);

// Class functions and variables
class TFT_CHAR : public TFT_GFX { friend class TFT_eSprite;

 //--------------------------------------- public ------------------------------------//
 public:

  TFT_CHAR();

  virtual void     drawChar(clip_t& clip, int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size);

  virtual int16_t  drawChar(clip_t& clip, uint16_t uniCode, int32_t x, int32_t y, uint8_t font);

  // Text rendering - value returned is the pixel width of the rendered text
  int16_t  drawNumber(wh_clip_t& clip, long intNumber, int32_t x, int32_t y, uint8_t font), // Draw integer using specified font number

           // Decimal is the number of decimal places to render
           // Use with setTextDatum() to position values on TFT, and setTextPadding() to blank old displayed values
           drawFloat(wh_clip_t& clip, float floatNumber, uint8_t decimal, int32_t x, int32_t y, uint8_t font), // Draw float using specified font number

           // Handle char arrays
           // Use with setTextDatum() to position string on TFT, and setTextPadding() to blank old displayed strings
           drawString(wh_clip_t& clip, const char *string, int32_t x, int32_t y, uint8_t font),  // Draw string using specified font number

           drawCentreString(wh_clip_t& clip, const char *string, int32_t x, int32_t y, uint8_t font),  // Deprecated, use setTextDatum() and drawString()
           drawRightString(wh_clip_t& clip, const char *string, int32_t x, int32_t y, uint8_t font);   // Deprecated, use setTextDatum() and drawString()

           // Used by Smooth font class to fetch a pixel colour for the anti-aliasing
  void     setCallback(getColorCallback getCol);

  // Text rendering and font handling support funtions
  void     setCursor(int16_t x, int16_t y),                 // Set cursor for tft.print()
           setCursor(int16_t x, int16_t y, uint8_t font);   // Set cursor and font number for tft.print()

  int16_t  getCursorX(void),                                // Read current cursor x position (moves with tft.print())
           getCursorY(void);                                // Read current cursor y position

  void     setTextColor(uint16_t color),                    // Set character (glyph) color only (background not over-written)
           setTextColor(uint16_t fgcolor, uint16_t bgcolor, bool bgfill = false),  // Set character (glyph) foreground and background colour, optional background fill for smooth fonts
           setTextSize(uint8_t size);                       // Set character size multiplier (this increases pixel size)

  void     setTextWrap(bool wrapX, bool wrapY = false);     // Turn on/off wrapping of text in TFT width and/or height

  void     setTextDatum(uint8_t datum);                     // Set text datum position (default is top left), see Section 6 above
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

  int16_t  textWidth(const char *string, uint8_t font),     // Returns pixel width of string in specified font
           fontHeight(int16_t font);                        // Returns pixel height of string in specified font

           // Used by library and Smooth font class to extract Unicode point codes from a UTF8 encoded string
  uint16_t decodeUTF8(uint8_t *buf, uint16_t *index, uint16_t remaining),
           decodeUTF8(uint8_t c);

           // Support function to UTF8 decode and draw characters piped through print stream
  size_t   write(wh_clip_t& clip, uint8_t utf8);
           // size_t   write(const uint8_t *buf, size_t len);

  uint16_t fontsLoaded(void); // Each bit in returned value represents a font type that is loaded - used for debug/error handling only

  uint32_t textcolor, textbgcolor;         // Text foreground and background colours

  uint32_t bitmap_fg, bitmap_bg;           // Bitmap foreground (bit=1) and background (bit=0) colours

  uint8_t  textfont,  // Current selected font number
           textsize,  // Current font size multiplier
           textdatum; // Text reference datum

  uint8_t  decoderState = 0;   // UTF8 decoder state        - not for user access
  uint16_t decoderBuffer;      // Unicode code-point buffer - not for user access

 protected:
                       // User sketch manages these via set/getAttribute()
  bool     _cp437;        // If set, use correct CP437 charset (default is ON)
  bool     _utf8;         // If set, use UTF-8 decoder in print stream 'write()' function (default ON)

  bool     _fillbg;    // Fill background flag (just for for smooth fonts at the moment)

 //--------------------------------------- private ------------------------------------//
 private:

  getColorCallback getColor = nullptr; // Smooth font callback function pointer

  int32_t  cursor_x, cursor_y, padX;       // Text cursor x,y and padding setting
  int32_t  bg_cursor_x;                    // Background fill cursor
  int32_t  last_cursor_x;                  // Previous text cursor position when fill used

  uint32_t fontsloaded;               // Bit field of fonts loaded

  uint8_t  glyph_ab,   // Smooth font glyph delta Y (height) above baseline
           glyph_bb;   // Smooth font glyph delta Y (height) below baseline

  bool     isDigits;   // adjust bounding box for numbers to reduce visual jiggling
  bool     textwrapX, textwrapY;  // If set, 'wrap' text at right and optionally bottom edge of display

#ifdef LOAD_GFXFF
  GFXfont  *gfxFont = 0;
#endif

/***************************************************************************************
**                         Section 9: TFT_eSPI class conditional extensions
***************************************************************************************/

// Load the Anti-aliased font extension
#ifdef SMOOTH_FONT
  #include "Extensions/Smooth_font.h"  // Loaded if SMOOTH_FONT is defined by user
#endif
};

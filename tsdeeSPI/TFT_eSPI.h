/*
  Top level

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

#include "TFT_Print.h"

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
// Default color definitions

// Next is a special 16-bit colour value that encodes to 8 bits
// and will then decode back to the same 16-bit value.
// Convenient for 8-bit and 16-bit transparent sprites.

//#define BLACK        RGB(    0,    0,    0)
#define NAVY         RGB(    0,    0, 0x7C)
#define BLUE         RGB(    0,    0, 0xFC)
#define DARK_GREEN   RGB(    0, 0x7C,    0)
#define DARK_CYAN    RGB(    0, 0x7C, 0x7C)
#define TEAL         RGB(    0, 0x80, 0x80)
#define GREEN        RGB(    0, 0xFC,    0)
#define CYAN         RGB(    0, 0xFC, 0xFC)
#define DIMM_CYAN    RGB( 0x08, 0x70, 0xC0)
#define DODGER_BLUE  RGB( 0x1C, 0x90, 0xFC)
#define CADET_BLUE   RGB( 0x5C, 0x9C, 0xA0)
#define AQUA         RGB( 0x68, 0xC0, 0xE0)
#define MAROON       RGB( 0x7C,    0,    0)
#define PURPLE       RGB( 0x7C,    0, 0x7C)
#define OLIVE        RGB( 0x7C, 0x7C,    0)
#define GRAY         RGB( 0x7C, 0x7C, 0x7C)
#define DARK_GRAY    RGB( 0x80, 0x80, 0x80)
#define LIME         RGB( 0x80, 0xFC,    0)
#define SKYBLUE      RGB( 0x88, 0xCC, 0xEC)
#define VIOLET       RGB( 0xA0, 0x30, 0xC8)
#define SILVER       RGB( 0xC0, 0xC0, 0x80)
//#define BROWN        RGB( 0x80, 0x28, 0x28)
#define BROWN        RGB( 0x94, 0x4C, 0x00)
#define GREEN_YELLOW RGB( 0xAC, 0xFC, 0x00)
#define LIGHT_GRAY   RGB( 0xD4, 0xD4, 0xD4)
#define LIGHT_CYAN   RGB( 0xE0, 0xFC, 0xFC)
#define RED          RGB( 0xFC,    0,    0)
#define MAGENTA      RGB( 0xFC,    0, 0xFC)
#define CORAL        RGB( 0xFC, 0x7C, 0x50)
#define PINK         RGB( 0xFC, 0x80, 0xC4)
//#define ORANGE       RGB( 0xFC, 0x98,    0)
#define ORANGE       RGB( 0xFC, 0xA4,    0)
#define GOLDEN       RGB( 0xFC, 0xD8,    0)
#define YELLOW       RGB( 0xFC, 0xFC,    0)
#define LIGHT_YELLOW RGB( 0xFC, 0xFC, 0xE0)
//#define WHITE        RGB( 0xFC, 0xFC, 0xFC)

//#define TFT_BLACK        BLACK
#define TFT_NAVY         NAVY
#define TFT_DARKGREEN    DARK_GREEN
//#define TFT_WHITE        WHITE
#define TFT_DARKCYAN     DARK_CYAN
#define TFT_MAROON       MAROON
#define TFT_PURPLE       PURPLE
#define TFT_OLIVE        OLIVE
#define TFT_LIGHTGREY    LIGHT_GRAY
#define TFT_DARKGREY     GRAY
#define TFT_BLUE         BLUE
#define TFT_GREEN        GREEN
#define TFT_CYAN         CYAN
#define TFT_RED          RED
#define TFT_MAGENTA      MAGENTA
#define TFT_YELLOW       YELLOW
#define TFT_ORANGE       ORANGE
#define TFT_GREENYELLOW  GREEN_YELLOW
#define TFT_PINK         PINK
#define TFT_BROWN        BROWN
#define TFT_GOLD         GOLDEN
#define TFT_SILVER       SILVER
#define TFT_SKYBLUE      SKYBLUE
#define TFT_VIOLET       VIOLET

#define TFT_TRANSPARENT DARK_GREEN

// Default palette for 4-bit colour sprites
static const rgb_t default_4bit_palette[] PROGMEM = {
  TFT_BLACK,    //  0  ^
  TFT_BROWN,    //  1  |
  TFT_RED,      //  2  |
  TFT_ORANGE,   //  3  |
  TFT_YELLOW,   //  4  Colours 0-9 follow the resistor colour code!
  TFT_GREEN,    //  5  |
  TFT_BLUE,     //  6  |
  TFT_PURPLE,   //  7  |
  TFT_DARKGREY, //  8  |
  TFT_WHITE,    //  9  v
  TFT_CYAN,     // 10  Blue+green mix
  TFT_MAGENTA,  // 11  Blue+red mix
  TFT_MAROON,   // 12  Darker red colour
  TFT_DARKGREEN,// 13  Darker green colour
  TFT_NAVY,     // 14  Darker blue colour
  TFT_PINK      // 15
};

/***************************************************************************************
**                         Section 7: Diagnostic support
***************************************************************************************/
// #define TFT_eSPI_DEBUG     // Switch on debug support serial messages  (not used yet)
// #define TFT_eSPI_FNx_DEBUG // Switch on debug support for function "x" (not used yet)

// This structure allows sketches to retrieve the user setup parameters at runtime
// by calling getSetup(), zero impact on code size unless used, mainly for diagnostics
typedef struct
{
String  version = TFT_ESPI_VERSION;
String  setup_info;  // Setup reference name available to use in a user setup
uint32_t setup_id;   // ID available to use in a user setup
int32_t esp;         // Processor code
uint8_t trans;       // SPI transaction support
uint8_t serial;      // Serial (SPI) or parallel
#ifndef GENERIC_PROCESSOR
uint8_t  port;       // SPI port
#endif
uint8_t overlap;     // ESP8266 overlap mode
uint8_t interface;   // Interface type

uint16_t tft_driver; // Hexadecimal code
uint16_t tft_width;  // Rotation 0 width and height
uint16_t tft_height;

uint8_t r0_x_offset; // Display offsets, not all used yet
uint8_t r0_y_offset;
uint8_t r1_x_offset;
uint8_t r1_y_offset;
uint8_t r2_x_offset;
uint8_t r2_y_offset;
uint8_t r3_x_offset;
uint8_t r3_y_offset;

int8_t pin_tft_mosi; // SPI pins
int8_t pin_tft_miso;
int8_t pin_tft_clk;
int8_t pin_tft_cs;

int8_t pin_tft_dc;   // Control pins
int8_t pin_tft_rd;
int8_t pin_tft_wr;
int8_t pin_tft_rst;

int8_t pin_tft_d0;   // Parallel port pins
int8_t pin_tft_d1;
int8_t pin_tft_d2;
int8_t pin_tft_d3;
int8_t pin_tft_d4;
int8_t pin_tft_d5;
int8_t pin_tft_d6;
int8_t pin_tft_d7;

int8_t pin_tft_led;
int8_t pin_tft_led_on;

int8_t pin_tch_cs;   // Touch chip select pin

int16_t tft_spi_freq;// TFT write SPI frequency
int16_t tft_rd_freq; // TFT read  SPI frequency
int16_t tch_spi_freq;// Touch controller read/write SPI frequency
} setup_t;

/***************************************************************************************
**                         Section 8: Class member and support functions
***************************************************************************************/

// Class functions and variables
class TFT_eSPI : public TFT_Print { friend class TFT_eSprite; // Sprite class has access to protected members

 //--------------------------------------- public ------------------------------------//
 public:

  TFT_eSPI();

  // These are virtual so the TFT_eSprite class can override them with sprite specific functions
  void     drawPixel(int32_t x, int32_t y, rgb_t color),
           drawLine(int32_t xs, int32_t ys, int32_t xe, int32_t ye, rgb_t color),
           drawFastVLine(int32_t x, int32_t y, int32_t h, rgb_t color),
           drawFastHLine(int32_t x, int32_t y, int32_t w, rgb_t color),
           fillRect(int32_t x, int32_t y, int32_t w, int32_t h, rgb_t color);

  virtual  int16_t height(void);
  virtual  int16_t width(void);

                   // Read the colour of a pixel at x,y and return value in 565 format
  uint16_t readPixel(int32_t x, int32_t y);

  // Change the origin position from the default top left
  // Note: setRotation, setViewport and resetViewport will revert origin to top left corner of screen/sprite
  void     setOrigin(int32_t x, int32_t y);
  int32_t  getOriginX(void);
  int32_t  getOriginY(void);

  // Viewport commands, see "Viewport_Demo" sketch
  void     setViewport(int32_t x, int32_t y, int32_t w, int32_t h, bool vpDatum = true);
  bool     checkViewport(int32_t x, int32_t y, int32_t w, int32_t h);
  int32_t  getViewportX(void);
  int32_t  getViewportY(void);
  int32_t  getViewportWidth(void);
  int32_t  getViewportHeight(void);
  bool     getViewportDatum(void);
  void     frameViewport(uint16_t color, int32_t w);
  void     resetViewport(void) override;

  // Graphics drawing
  void     fillScreen(rgb_t color),
           drawRect(int32_t x, int32_t y, int32_t w, int32_t h, rgb_t color),
           drawRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, rgb_t color),
           fillRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, rgb_t color);

  void     fillRectVGradient(int32_t x, int32_t y, int32_t w, int32_t h, rgb_t color1, rgb_t color2);
  void     fillRectHGradient(int32_t x, int32_t y, int32_t w, int32_t h, rgb_t color1, rgb_t color2);

  void     drawCircle(int32_t x, int32_t y, int32_t r, rgb_t color),
           drawCircleHelper(int32_t x, int32_t y, int32_t r, uint8_t cornername, rgb_t color),
           fillCircle(int32_t x, int32_t y, int32_t r, rgb_t color),
           fillCircleHelper(int32_t x, int32_t y, int32_t r, uint8_t cornername, int32_t delta, rgb_t color),

           drawEllipse(int16_t x, int16_t y, int32_t rx, int32_t ry, uint16_t color),
           fillEllipse(int16_t x, int16_t y, int32_t rx, int32_t ry, uint16_t color),

           //                 Corner 1               Corner 2               Corner 3
           drawTriangle(int32_t x1,int32_t y1, int32_t x2,int32_t y2, int32_t x3,int32_t y3, rgb_t color),
           fillTriangle(int32_t x1,int32_t y1, int32_t x2,int32_t y2, int32_t x3,int32_t y3, rgb_t color);


  // Smooth (anti-aliased) graphics drawing
           // Draw a pixel blended with the background pixel colour (bg_color) specified,  return blended colour
           // If the bg_color is not specified, the background pixel colour will be read from TFT or sprite
  uint16_t drawPixel(int32_t x, int32_t y, rgb_t color, uint8_t alpha, rgb_t bg_color = WHITE);

  using TFT_eeSPI::drawPixel;

           // Draw an anti-aliased (smooth) arc between start and end angles. Arc ends are anti-aliased.
           // By default the arc is drawn with square ends unless the "roundEnds" parameter is included and set true
           // Angle = 0 is at 6 o'clock position, 90 at 9 o'clock etc. The angles must be in range 0-360 or they will be clipped to these limits
           // The start angle may be larger than the end angle. Arcs are always drawn clockwise from the start angle.
  void     drawSmoothArc(int32_t x, int32_t y, int32_t r, int32_t ir, rgb_t startAngle, rgb_t endAngle, rgb_t fg_color, rgb_t bg_color, bool roundEnds = false);

           // As per "drawSmoothArc" except the ends of the arc are NOT anti-aliased, this facilitates dynamic arc length changes with
           // arc segments and ensures clean segment joints. 
           // The sides of the arc are anti-aliased by default. If smoothArc is false sides will NOT be anti-aliased
  void     drawArc(int32_t x, int32_t y, int32_t r, int32_t ir, uint32_t startAngle, uint32_t endAngle, rgb_t fg_color, rgb_t bg_color, bool smoothArc = true);

           // Draw an anti-aliased filled circle at x, y with radius r
           // Note: The thickness of line is 3 pixels to reduce the visible "braiding" effect of anti-aliasing narrow lines
           //       this means the inner anti-alias zone is always at r-1 and the outer zone at r+1
  void     drawSmoothCircle(int32_t x, int32_t y, int32_t r, rgb_t fg_color, rgb_t bg_color);
  
           // Draw an anti-aliased filled circle at x, y with radius r
           // If bg_color is not included the background pixel colour will be read from TFT or sprite
  void     fillSmoothCircle(int32_t x, int32_t y, int32_t r, rgb_t color, rgb_t bg_color = WHITE);

           // Draw a rounded rectangle that has a line thickness of r-ir+1 and bounding box defined by x,y and w,h
           // The outer corner radius is r, inner corner radius is ir
           // The inside and outside of the border are anti-aliased
  void     drawSmoothRoundRect(int32_t x, int32_t y, int32_t r, int32_t ir, int32_t w, int32_t h, rgb_t fg_color, rgb_t bg_color = WHITE, uint8_t quadrants = 0xF);

           // Draw a filled rounded rectangle , corner radius r and bounding box defined by x,y and w,h
  void     fillSmoothRoundRect(int32_t x, int32_t y, int32_t w, int32_t h, int32_t radius, rgb_t color, rgb_t bg_color = WHITE);

           // Draw a small anti-aliased filled circle at ax,ay with radius r (uses drawWideLine)
           // If bg_color is not included the background pixel colour will be read from TFT or sprite
  void     drawSpot(float ax, float ay, float r, rgb_t fg_color, rgb_t bg_color = WHITE);

           // Draw an anti-aliased wide line from ax,ay to bx,by width wd with radiused ends (radius is wd/2)
           // If bg_color is not included the background pixel colour will be read from TFT or sprite
  void     drawWideLine(float ax, float ay, float bx, float by, float wd, rgb_t fg_color, rgb_t bg_color = WHITE);

           // Draw an anti-aliased wide line from ax,ay to bx,by with different width at each end aw, bw and with radiused ends
           // If bg_color is not included the background pixel colour will be read from TFT or sprite
  void     drawWedgeLine(float ax, float ay, float bx, float by, float aw, float bw, rgb_t fg_color, rgb_t bg_color = WHITE);


  // Image rendering
           // Draw bitmap
  void     drawBitmap( int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t fgcolor),
           drawBitmap( int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t fgcolor, uint16_t bgcolor),
           drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t fgcolor),
           drawXBitmap(int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t fgcolor, uint16_t bgcolor);

           // Set TFT pivot point (use when rendering rotated sprites)
  void     setPivot(int16_t x, int16_t y);
  int16_t  getPivotX(void), // Get pivot x
           getPivotY(void); // Get pivot y

           // The next functions can be used as a pair to copy screen blocks (or horizontal/vertical lines) to another location
           // Read a block of pixels to a data buffer, buffer is 16-bit and the size must be at least w * h
  void     readRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data);
           // Write a block of pixels to the screen which have been read by readRect()
  void     pushRect(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data);

           // These are used to render images or sprites stored in RAM arrays (used by Sprite class for 16bpp Sprites)
  void     pushImage16(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data);
  void     pushImage16(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data, rgb_t transparent);

           // These are used to render images stored in FLASH (PROGMEM)
  void     pushImage16(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data, rgb_t transparent);
  void     pushImage16(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data);

           // These are used by Sprite class pushSprite() member function for 1, 4 and 8 bits per pixel (bpp) colours
           // They are not intended to be used with user sketches (but could be)
           // Set bpp8 true for 8bpp sprites, false otherwise. The cmap pointer must be specified for 4bpp
  void     pushImage16(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t  *data, bool bpp8, cmap_t& cmap);
  void     pushImage16(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t  *data, rgb_t transp, bool bpp8, cmap_t& cmap);
           // FLASH version
  void     pushImage16(int32_t x, int32_t y, int32_t w, int32_t h, const uint8_t *data, bool bpp8, cmap_t& cmap);

           // Render a 16-bit colour image with a 1bpp mask
  void     pushMaskedImage16(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *img, uint8_t *mask);

           // This next function has been used successfully to dump the TFT screen to a PC for documentation purposes
           // It reads a screen area and returns the 3 RGB 8-bit colour values of each pixel in the buffer
           // Set w and h to 1 to read 1 pixel's colour. The data buffer must be at least w * h * 3 bytes
  void     readRectRGB(int32_t x, int32_t y, int32_t w, int32_t h, uint8_t *data);

  void     pushImageDMA16(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t* data, uint16_t* buffer = nullptr);

  // Set/get an arbitrary library configuration attribute or option
  //       Use to switch ON/OFF capabilities such as UTF8 decoding - each attribute has a unique ID
  //       id = 0: reserved - may be used in future to reset all attributes to a default state
  //       id = 1: Turn on (a=true) or off (a=false) GLCD cp437 font character error correction
  //       id = 2: Turn on (a=true) or off (a=false) UTF8 decoding
  //       id = 3: Enable or disable use of ESP32 PSRAM (if available)
           #define CP437_SWITCH 1
           #define UTF8_SWITCH  2
           #define PSRAM_ENABLE 3
  void     setAttribute(uint8_t id = 0, uint8_t a = 0); // Set attribute value
  uint8_t  getAttribute(uint8_t id = 0);                // Get attribute value

           // Used for diagnostic sketch to see library setup adopted by compiler, see Section 7 above
  void     getSetup(setup_t& tft_settings); // Sketch provides the instance to populate
  bool     verifySetupID(uint32_t id);

#ifdef SMOOTH_FONT
  void     showFont(uint32_t td);
#endif

#ifdef COLOR_565
  #define pushBlock          pushBlock16
  #define pushPixels         pushPixels16
  #define pushPixelsDMA      pushPixelsDMA16
  #define pushImage          pushImage16
  #define pushMaskedImage    pushMaskedImage16
  #define pushImageDMA       pushImageDMA16
#else
#endif

  using TFT_eeSPI::readRectRGB;

  using TFT_GFX::drawPixel;
  using TFT_GFX::readPixel;
  using TFT_GFX::readRect;
  using TFT_GFX::drawFastVLine;
  using TFT_GFX::drawFastHLine;
  using TFT_GFX::drawLine;
  using TFT_GFX::fillRect;
  using TFT_GFX::drawRect;
  using TFT_GFX::fillRectVGradient;
  using TFT_GFX::fillRectHGradient;
  using TFT_GFX::drawRoundRect;
  using TFT_GFX::fillRoundRect;
  using TFT_GFX::drawCircle;
  using TFT_GFX::drawCircleHelper;
  using TFT_GFX::fillCircle;
  using TFT_GFX::fillCircleHelper;
  using TFT_GFX::drawEllipse;
  using TFT_GFX::fillEllipse;
  using TFT_GFX::drawTriangle;
  using TFT_GFX::fillTriangle;
  using TFT_GFX::drawArc;
  using TFT_GFX::drawSmoothArc;
  using TFT_GFX::drawSmoothCircle;
  using TFT_GFX::fillSmoothCircle;
  using TFT_GFX::drawSmoothRoundRect;
  using TFT_GFX::fillSmoothRoundRect;
  using TFT_GFX::drawWedgeLine;
  using TFT_GFX::drawBitmap;
  using TFT_GFX::drawXBitmap;
  using TFT_GFX::pushRect;
  using TFT_GFX::pushImage16;
  using TFT_GFX::pushMaskedImage16;
  using TFT_GFX::pushImageDMA16;
  using TFT_GFX::pushPixels16;
  using TFT_GFX::pushPixelsDMA16;

  //-------------------------------------- protected ----------------------------------//
 protected:

  //int32_t  win_xe, win_ye;          // Window end coords - not needed

  int16_t  _xPivot;   // TFT x pivot point coordinate for rotated Sprites
  int16_t  _yPivot;   // TFT x pivot point coordinate for rotated Sprites

  // Viewport variables
//  int32_t  _xDatum;
//  int32_t  _yDatum;
//  int32_t  _width, _height;           // Display w/h as modified by current rotation
  int32_t  _xWidth;
  int32_t  _yHeight;

/***************************************************************************************
**                         Section 9: TFT_eSPI class conditional extensions
***************************************************************************************/

}; // End of class TFT_eSPI

/***************************************************************************************
**                         Section 10: Additional extension classes
***************************************************************************************/
// Load the Button Class
#include "Extensions/Button.h"

// Load the Sprite Class
#include "Extensions/Sprite.h"


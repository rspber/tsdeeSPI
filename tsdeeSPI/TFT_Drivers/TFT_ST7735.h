
#pragma once

// Change the width and height if required (defined in portrait mode)
// or use the constructor to over-ride defaults
#ifndef TFT_WIDTH
  #define TFT_WIDTH  128
#endif
#ifndef TFT_HEIGHT
  #define TFT_HEIGHT 160
#endif


// Enumerate the different configurations
#define INITR_GREENTAB       0x0
#define INITR_REDTAB         0x1
#define INITR_BLACKTAB       0x2 // Display with no offsets
#define INITR_GREENTAB2      0x3 // Use if you get random pixels on two edges of green tab display
#define INITR_GREENTAB3      0x4 // Use if you get random pixels on edge(s) of 128x128 screen
#define INITR_GREENTAB128    0x5 // Use if you only get part of 128x128 screen in rotation 0 & 1
#define INITR_GREENTAB160x80 0x6 // Use if you only get part of 128x128 screen in rotation 0 & 1
#define INITR_REDTAB160x80   0x7 // Added for https://www.aliexpress.com/item/ShengYang-1pcs-IPS-0-96-inch-7P-SPI-HD-65K-Full-Color-OLED-Module-ST7735-Drive/32918394604.html
#define INITR_ROBOTLCD       0x8
#define INITB                0xB


// Setup the tab color that will be used by the library setRotation() and setup command list
#if defined (ST7735_INITB)
  #define TAB_COLOUR INITB
  
#elif defined (ST7735_GREENTAB)
  #define TAB_COLOUR INITR_GREENTAB
  #define CGRAM_OFFSET
  
#elif defined (ST7735_GREENTAB2)
  #define TAB_COLOUR INITR_GREENTAB2
  #define CGRAM_OFFSET

#elif defined (ST7735_GREENTAB3)
  #define TAB_COLOUR INITR_GREENTAB3
  #define CGRAM_OFFSET

#elif defined (ST7735_GREENTAB128)
  #define TAB_COLOUR INITR_GREENTAB128
  #define CGRAM_OFFSET
 
#elif defined (ST7735_GREENTAB160x80)
  #define TAB_COLOUR INITR_GREENTAB160x80
  #define CGRAM_OFFSET

#elif defined (ST7735_ROBOTLCD)
  #define TAB_COLOUR INITR_ROBOTLCD
  #define CGRAM_OFFSET

#elif defined (ST7735_REDTAB160x80)
  #define TAB_COLOUR INITR_REDTAB160x80
  #define CGRAM_OFFSET
 
#elif defined (ST7735_REDTAB)
  #define TAB_COLOUR INITR_REDTAB
  
#elif defined (ST7735_BLACKTAB)
  #define TAB_COLOUR INITR_BLACKTAB

#else // Make sure it is not undefined
  #define TAB_COLOUR INITR_BLACKTAB
#endif


// ST7735 specific commands used in init
#define ST7735_RDDID   0x04
#define ST7735_RDDST   0x09

#define ST7735_FRMCTR1 0xB1
#define ST7735_FRMCTR2 0xB2
#define ST7735_FRMCTR3 0xB3
#define ST7735_INVCTR  0xB4
#define ST7735_DISSET5 0xB6

#define ST7735_PWCTR1  0xC0
#define ST7735_PWCTR2  0xC1
#define ST7735_PWCTR3  0xC2
#define ST7735_PWCTR4  0xC3
#define ST7735_PWCTR5  0xC4
#define ST7735_VMCTR1  0xC5

#define ST7735_RDID1   0xDA
#define ST7735_RDID2   0xDB
#define ST7735_RDID3   0xDC
#define ST7735_RDID4   0xDD

#define ST7735_PWCTR6  0xFC

#define ST7735_GMCTRP1 0xE0
#define ST7735_GMCTRN1 0xE1

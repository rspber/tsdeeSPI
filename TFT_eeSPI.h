/*
  TFT low level procedures

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

#define TFT_ESPI_VERSION "2.5.34"

// Bit level feature flags
// Bit 0 set: viewport capability
#define TFT_ESPI_FEATURES 1

/***************************************************************************************
**                         Section 1: Load required header files
***************************************************************************************/

//Standard support
#include <Arduino.h>
#include <Print.h>
#if !defined (TFT_PARALLEL_8_BIT) && !defined (RP2040_PIO_INTERFACE)
  #include <SPI.h>
#endif

/***************************************************************************************
**                         Section 2: Load library and processor specific header files
***************************************************************************************/
// Include header file that defines the fonts loaded, the TFT drivers
// available and the pins to be used, etc, etc
#ifdef CONFIG_TFT_eSPI_ESPIDF
  #include "TFT_config.h"
#endif

// New ESP8266 board package uses ARDUINO_ARCH_ESP8266
// old package defined ESP8266
#if defined (ESP8266)
  #ifndef ARDUINO_ARCH_ESP8266
    #define ARDUINO_ARCH_ESP8266
  #endif
#endif

// The following lines allow the user setup to be included in the sketch folder, see
// "Sketch_with_tft_setup" generic example.
#if !defined __has_include
  #if !defined(DISABLE_ALL_LIBRARY_WARNINGS)
    #warning Compiler does not support __has_include, so sketches cannot define the setup
  #endif
#else
  #if __has_include(<tft_setup.h>)
    // Include the sketch setup file
    #include <tft_setup.h>
    #ifndef USER_SETUP_LOADED
      // Prevent loading further setups
      #define USER_SETUP_LOADED
    #endif
  #endif
#endif

#include <User_Setup_Select.h>

// Handle FLASH based storage e.g. PROGMEM
#if defined(ARDUINO_ARCH_RP2040)
  #undef pgm_read_byte
  #define pgm_read_byte(addr)   (*(const unsigned char *)(addr))
  #undef pgm_read_word
  #define pgm_read_word(addr) ({ \
    typeof(addr) _addr = (addr); \
    *(const unsigned short *)(_addr); \
  })
  #undef pgm_read_dword
  #define pgm_read_dword(addr) ({ \
    typeof(addr) _addr = (addr); \
    *(const unsigned long *)(_addr); \
  })
#elif defined(__AVR__)
  #include <avr/pgmspace.h>
#elif defined(ARDUINO_ARCH_ESP8266) || defined(ESP32)
  #include <pgmspace.h>
#else
  #ifndef PROGMEM
    #define PROGMEM
  #endif
#endif

// Include the processor specific drivers
#if defined(CONFIG_IDF_TARGET_ESP32S3)
  #include "Processors/TFT_eSPI_ESP32_S3.h"
#elif defined(CONFIG_IDF_TARGET_ESP32C3)
  #include "Processors/TFT_eSPI_ESP32_C3.h"
#elif defined (ESP32)
  #include "Processors/TFT_eSPI_ESP32.h"
#elif defined (ARDUINO_ARCH_ESP8266)
  #include "Processors/TFT_eSPI_ESP8266.h"
#elif defined (STM32)
  #include "Processors/TFT_eSPI_STM32.h"
#elif defined(ARDUINO_ARCH_RP2040)
  #include "Processors/TFT_eSPI_RP2040.h"
#else
  #include "Processors/TFT_eSPI_Generic.h"
  #define GENERIC_PROCESSOR
#endif

/***************************************************************************************
**                         Section 3: Interface setup
***************************************************************************************/
#ifndef TAB_COLOUR
  #define TAB_COLOUR 0
#endif

// If the SPI frequency is not defined, set a default
#ifndef SPI_FREQUENCY
  #define SPI_FREQUENCY  20000000
#endif

// If the SPI read frequency is not defined, set a default
#ifndef SPI_READ_FREQUENCY
  #define SPI_READ_FREQUENCY 10000000
#endif

// Some ST7789 boards do not work with Mode 0
#ifndef TFT_SPI_MODE
  #if defined(ST7789_DRIVER) || defined(ST7789_2_DRIVER)
    #define TFT_SPI_MODE SPI_MODE3
  #else
    #define TFT_SPI_MODE SPI_MODE0
  #endif
#endif

// If the XPT2046 SPI frequency is not defined, set a default
#ifndef SPI_TOUCH_FREQUENCY
  #define SPI_TOUCH_FREQUENCY  2500000
#endif

#ifndef SPI_BUSY_CHECK
  #define SPI_BUSY_CHECK
#endif

// If half duplex SDA mode is defined then MISO pin should be -1
#ifdef TFT_SDA_READ
  #ifdef TFT_MISO
    #if TFT_MISO != -1
      #undef TFT_MISO
      #define TFT_MISO -1
      #warning TFT_MISO set to -1
    #endif
  #endif
#endif

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

#include "aclip.h"

// Class functions and variables
class TFT_eeSPI : public Print { friend class TFT_eSprite; // Sprite class has access to protected members

  friend class TFT_GFX;
  friend class TFT_CHAR;

 //--------------------------------------- public ------------------------------------//
 public:

  TFT_eeSPI();

  // init() and begin() are equivalent, begin() included for backwards compatibility
  // Sketch defined tab colour option is for ST7735 displays only
  void     init(uint8_t tc = TAB_COLOUR), begin(uint8_t tc = TAB_COLOUR);

  virtual void     resetViewport(void) {}

  // These are virtual so the TFT_eSprite class can override them with sprite specific functions
  virtual void     drawPixel(clip_t& clip, int32_t x, int32_t y, uint32_t color);

                   // Read the colour of a pixel at x,y and return value in 565 format
  virtual uint16_t readPixel(clip_t& clip, int32_t x, int32_t y);

  virtual void     setWindow(int32_t x, int32_t y, int32_t w, int32_t h);   // Note: start + end coordinates

                   // Push (aka write pixel) colours to the set window
  virtual void     pushColor(uint16_t color);

                   // These are non-inlined to enable override
  virtual void     begin_nin_write();
  virtual void     end_nin_write();

  void     setRotation(uint8_t r); // Set the display image orientation to 0, 1, 2 or 3
  uint8_t  getRotation(void);      // Read the current rotation

  virtual void     setRotationSizes(uint8_t r) {};

  void     invertDisplay(bool i);  // Tell TFT to invert all displayed colours


  // The TFT_eSprite class inherits the following functions (not all are useful to Sprite class
  void     setAddrWindow(int32_t xs, int32_t ys, int32_t w, int32_t h); // Note: start coordinates + width and height

           // Push (aka write pixel) colours to the TFT (use setAddrWindow() first)
  void     pushColor(uint16_t color, uint32_t len),  // Deprecated, use pushBlock()
           pushColors(uint16_t  *data, uint32_t len, bool swapBytes = true), // With byte swap option
           pushColors(uint8_t  *data, uint32_t len, bool swapBytes); // Deprecated, use pushPixels()

           // Write a solid block of a single colour
  void     pushBlock(uint16_t color, uint32_t len);

           // Write a set of pixels stored in memory, use setSwapBytes(true/false) function to correct endianess
  void     pushPixels(const void * data_in, uint32_t len, bool swapBytes);

           // Support for half duplex (bi-directional SDA) SPI bus where MOSI must be switched to input
           #ifdef TFT_SDA_READ
             #if defined (TFT_eSPI_ENABLE_8_BIT_READ)
  uint8_t  tft_Read_8(void);     // Read 8 bit value from TFT command register
             #endif
  void     begin_SDA_Read(void); // Begin a read on a half duplex (bi-directional SDA) SPI bus - sets MOSI to input
  void     end_SDA_Read(void);   // Restore MOSI to output
           #endif

           // The next functions can be used as a pair to copy screen blocks (or horizontal/vertical lines) to another location
           // Read a block of pixels to a data buffer, buffer is 16 bit and the size must be at least w * h
  void     readRect(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data);

           // This next function has been used successfully to dump the TFT screen to a PC for documentation purposes
           // It reads a screen area and returns the 3 RGB 8 bit colour values of each pixel in the buffer
           // Set w and h to 1 to read 1 pixel's colour. The data buffer must be at least w * h * 3 bytes
  void     readRectRGB(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, uint8_t *data);

   // Low level read/write
  void     spiwrite(uint8_t);        // legacy support only
#ifdef RM68120_DRIVER
  void     writecommand(uint16_t c);                 // Send a 16 bit command, function resets DC/RS high ready for data
  void     writeRegister8(uint16_t c, uint8_t d);    // Write 8 bit data data to 16 bit command register
  void     writeRegister16(uint16_t c, uint16_t d);  // Write 16 bit data data to 16 bit command register
#else
  void     writecommand(uint8_t c);  // Send an 8 bit command, function resets DC/RS high ready for data
#endif
  void     writedata(uint8_t d);     // Send data with DC/RS set high

  void     commandList(const uint8_t *addr); // Send a initialisation sequence to TFT stored in FLASH

  uint8_t  readcommand8( uint8_t cmd_function, uint8_t index = 0); // read 8 bits from TFT
  uint16_t readcommand16(uint8_t cmd_function, uint8_t index = 0); // read 16 bits from TFT
  uint32_t readcommand32(uint8_t cmd_function, uint8_t index = 0); // read 32 bits from TFT

           // Convert 8 bit red, green and blue to 16 bits
  uint16_t color565(uint8_t red, uint8_t green, uint8_t blue);

  // Direct Memory Access (DMA) support functions
  // These can be used for SPI writes when using the ESP32 (original) or STM32 processors.
  // DMA also works on a RP2040 processor with PIO based SPI and parallel (8 and 16 bit) interfaces
           // Bear in mind DMA will only be of benefit in particular circumstances and can be tricky
           // to manage by noobs. The functions have however been designed to be noob friendly and
           // avoid a few DMA behaviour "gotchas".
           //
           // At best you will get a 2x TFT rendering performance improvement when using DMA because
           // this library handles the SPI bus so efficiently during normal (non DMA) transfers. The best
           // performance improvement scenario is the DMA transfer time is exactly the same as the time it
           // takes for the processor to prepare the next image buffer and initiate another DMA transfer.
           //
           // DMA transfer to the TFT is done while the processor moves on to handle other tasks. Bear
           // this in mind and watch out for "gotchas" like the image buffer going out of scope as the
           // processor leaves a function or its content being changed while the DMA engine is reading it.
           //
           // The compiler MAY change the implied scope of a buffer which has been set aside by creating
           // an array. For example a buffer defined before a "for-next" loop may get de-allocated when
           // the loop ends. To avoid this use, for example, malloc() and free() to take control of when
           // the buffer space is available and ensure it is not released until DMA is complete.
           //
           // Clearly you should not modify a buffer that is being DMA'ed to the TFT until the DMA is over.
           // Use the dmaBusy() function to check this.  Use tft.startWrite() before invoking DMA so the
           // TFT chip select stays low. If you use tft.endWrite() before DMA is complete then the endWrite
           // function will wait for the DMA to complete, so this may defeat any DMA performance benefit.
           //

  bool     initDMA(bool ctrl_cs = false);  // Initialise the DMA engine and attach to SPI bus - typically used in setup()
                                           // Parameter "true" enables DMA engine control of TFT chip select (ESP32 only)
                                           // For ESP32 only, TFT reads will not work if parameter is true
  void     deInitDMA(void);   // De-initialise the DMA engine and detach from SPI bus - typically not used

           // Push an image to the TFT using DMA, buffer is optional and grabs (double buffers) a copy of the image
           // Use the buffer if the image data will get over-written or destroyed while DMA is in progress
           //
           // Note 1: If swapping colour bytes is defined, and the double buffer option is NOT used, then the bytes
           // in the original image buffer content will be byte swapped by the function before DMA is initiated.
           //
           // Note 2: If part of the image will be off screen or outside of a set viewport, then the the original
           // image buffer content will be altered to a correctly clipped image before DMA is initiated.
           //
           // The function will wait for the last DMA to complete if it is called while a previous DMA is still
           // in progress, this simplifies the sketch and helps avoid "gotchas".
  void     pushImageDMA(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, bool swapBytes, uint16_t* data, uint16_t* buffer);

#if defined (ESP32) // ESP32 only at the moment
           // For case where pointer is a const and the image data must not be modified (clipped or byte swapped)
  void     pushImageDMA(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t const* data);
#endif
           // Push a block of pixels into a window set up using setAddrWindow()
  void     pushPixelsDMA(uint16_t* image, uint32_t len, bool swapBytes);

           // Check if the DMA is complete - use while(tft.dmaBusy); for a blocking wait
  bool     dmaBusy(void); // returns true if DMA is still in progress
  void     dmaWait(void); // wait until DMA is complete

  bool     DMA_Enabled = false;   // Flag for DMA enabled state
  uint8_t  spiBusyCheck = 0;      // Number of ESP32 transfer buffers to check

  // Bare metal functions
  void     startWrite(void);                         // Begin SPI transaction
  void     writeColor(uint16_t color, uint32_t len); // Deprecated, use pushBlock()
  void     endWrite(void);                           // End SPI transaction


  // Global variables
#if !defined (TFT_PARALLEL_8_BIT) && !defined (RP2040_PIO_INTERFACE)
  static   SPIClass& getSPIinstance(void); // Get SPI class handle
#endif

  uint8_t rotation;  // Display rotation (0-3)

 //--------------------------------------- private ------------------------------------//
 private:
           // Legacy begin and end prototypes - deprecated TODO: delete
  void     spi_begin();
  void     spi_end();

  void     spi_begin_read();
  void     spi_end_read();

           // New begin and end prototypes
           // begin/end a TFT write transaction
           // For SPI bus the transmit clock rate is set
  void begin_tft_write();
  void end_tft_write();
  //inline void begin_tft_write() __attribute__((always_inline));
  //inline void end_tft_write()   __attribute__((always_inline));

           // begin/end a TFT read transaction
           // For SPI bus: begin lowers SPI clock rate, end reinstates transmit clock rate
  inline void begin_tft_read()  __attribute__((always_inline));
  inline void end_tft_read()    __attribute__((always_inline));

           // Initialise the data bus GPIO and hardware interfaces
  void     initBus(void);

           // Temporary  library development function  TODO: remove need for this
  void     pushSwapBytePixels(const void* data_in, uint32_t len);

           // Same as setAddrWindow but exits with CGRAM in read mode
  void     readAddrWindow(int32_t xs, int32_t ys, int32_t w, int32_t h);

           // Byte read prototype
  uint8_t  readByte(void);

           // GPIO parallel bus input/output direction control
  void     busDir(uint32_t mask, uint8_t mode);

           // Single GPIO input/output direction control
  void     gpioMode(uint8_t gpio, uint8_t mode);

           // Port and pin masks for control signals (ESP826 only) - TODO: remove need for this
  volatile uint32_t *dcport, *csport;
  uint32_t cspinmask, dcpinmask, wrpinmask, sclkpinmask;

           #if defined(ESP32_PARALLEL)
           // Bit masks for ESP32 parallel bus interface
  uint32_t xclr_mask, xdir_mask; // Port set/clear and direction control masks

           // Lookup table for ESP32 parallel bus interface uses 1kbyte RAM,
  uint32_t xset_mask[256]; // Makes Sprite rendering test 33% faster, for slower macro equivalent
                           // see commented out #define set_mask(C) within TFT_eSPI_ESP32.h
           #endif

  bool     locked, inTransaction, lockTransaction; // SPI transaction and mutex lock flags

 //-------------------------------------- protected ----------------------------------//
 protected:

  int32_t  addr_row, addr_col;        // Window position - used to minimise window commands

  bool     _booted;    // init() or begin() has already run once

  bool     _psram_enable; // Enable PSRAM use for library functions (TBD) and Sprites

  void drawCharDefault(const uint8_t* font_offset, uint16_t color, uint16_t bg);
  void drawCharFaster(uint16_t width, uint16_t height, uint16_t textcolor, uint16_t textbgcolor, uint32_t flash_address);
  void drawRLEfont(int32_t xd, int32_t y, int32_t pY, uint16_t width, uint16_t height, uint16_t textcolor, int16_t textsize, uint32_t flash_addres);

#if defined (SSD1963_DRIVER)
  uint16_t Cswap;      // Swap buffer for SSD1963
  uint8_t r6, g6, b6;  // RGB buffer for SSD1963
#endif

/***************************************************************************************
**                         Section 9: TFT_eSPI class conditional extensions
***************************************************************************************/
// Load the Touch extension
#ifdef TOUCH_CS
  #if defined (TFT_PARALLEL_8_BIT) || defined (RP2040_PIO_INTERFACE)
    #if !defined(DISABLE_ALL_LIBRARY_WARNINGS)
      #error >>>>------>> Touch functions not supported in 8/16 bit parallel mode or with RP2040 PIO.
    #endif
  #else
    #include "Extensions/Touch.h"        // Loaded if TOUCH_CS is defined by user
  #endif
#else
    #if !defined(DISABLE_ALL_LIBRARY_WARNINGS)
      #warning >>>>------>> TOUCH_CS pin not defined, TFT_eSPI touch functions will not be available!
    #endif
#endif

}; // End of class TFT_eeSPI

// Swap any type
template <typename T> static inline void
transpose(T& a, T& b) { T t = a; a = b; b = t; }

/***************************************************************************************
**                         Section 10: Additional extension classes
***************************************************************************************/

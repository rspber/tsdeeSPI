/*
  TFT low level procedures

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

#include "TFT_eeSPI.h"

#if defined (ESP32)
  #if defined(CONFIG_IDF_TARGET_ESP32S3)
    #include "Processors/TFT_eSPI_ESP32_S3.c" // Tested with SPI and 8-bit parallel
  #elif defined(CONFIG_IDF_TARGET_ESP32C3)
    #include "Processors/TFT_eSPI_ESP32_C3.c" // Tested with SPI (8-bit parallel will probably work too!)
  #else
    #include "Processors/TFT_eSPI_ESP32.c"
  #endif
#elif defined (ARDUINO_ARCH_ESP8266)
  #include "Processors/TFT_eSPI_ESP8266.c"
#elif defined (STM32) // (_VARIANT_ARDUINO_STM32_) stm32_def.h
  #include "Processors/TFT_eSPI_STM32.c"
#elif defined (ARDUINO_ARCH_RP2040)  || defined (ARDUINO_ARCH_MBED) // Raspberry Pi Pico
  #include "Processors/TFT_eSPI_RP2040.c"
#else
  #include "Processors/TFT_eSPI_Generic.c"
#endif

#ifndef SPI_BUSY_CHECK
  #define SPI_BUSY_CHECK
#endif


/***************************************************************************************
** Function name:           Legacy - deprecated
** Description:             Start/end transaction
***************************************************************************************/
  void TFT_eeSPI::spi_begin()       {begin_tft_write();}
  void TFT_eeSPI::spi_end()         {  end_tft_write();}
  void TFT_eeSPI::spi_begin_read()  {begin_tft_read(); }
  void TFT_eeSPI::spi_end_read()    {  end_tft_read(); }

/***************************************************************************************
** Function name:           begin_tft_write (was called spi_begin)
** Description:             Start SPI transaction for writes and select TFT
***************************************************************************************/
void TFT_eeSPI::begin_tft_write(void){
  if (locked) {
    locked = false; // Flag to show SPI access now unlocked
#if defined (SPI_HAS_TRANSACTION) && defined (SUPPORT_TRANSACTIONS) && !defined(TFT_PARALLEL_8_BIT) && !defined(RP2040_PIO_INTERFACE)
    spi.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, TFT_SPI_MODE));
#endif
    CS_L;
    SET_BUS_WRITE_MODE;  // Some processors (e.g. ESP32) allow recycling the tx buffer when rx is not used
  }
}

// Non-inlined version to permit override
void TFT_eeSPI::begin_nin_write(void){
  if (locked) {
    locked = false; // Flag to show SPI access now unlocked
#if defined (SPI_HAS_TRANSACTION) && defined (SUPPORT_TRANSACTIONS) && !defined(TFT_PARALLEL_8_BIT) && !defined(RP2040_PIO_INTERFACE)
    spi.beginTransaction(SPISettings(SPI_FREQUENCY, MSBFIRST, TFT_SPI_MODE));
#endif
    CS_L;
    SET_BUS_WRITE_MODE;  // Some processors (e.g. ESP32) allow recycling the tx buffer when rx is not used
  }
}

/***************************************************************************************
** Function name:           end_tft_write (was called spi_end)
** Description:             End transaction for write and deselect TFT
***************************************************************************************/
void TFT_eeSPI::end_tft_write(void){
  if(!inTransaction) {      // Flag to stop ending transaction during multiple graphics calls
    if (!locked) {          // Locked when beginTransaction has been called
      locked = true;        // Flag to show SPI access now locked
      SPI_BUSY_CHECK;       // Check send complete and clean out unused rx data
      CS_H;
      SET_BUS_READ_MODE;    // In case bus has been configured for tx only
#if defined (SPI_HAS_TRANSACTION) && defined (SUPPORT_TRANSACTIONS) && !defined(TFT_PARALLEL_8_BIT) && !defined(RP2040_PIO_INTERFACE)
      spi.endTransaction();
#endif
    }
  }
}

// Non-inlined version to permit override
inline void TFT_eeSPI::end_nin_write(void){
  if(!inTransaction) {      // Flag to stop ending transaction during multiple graphics calls
    if (!locked) {          // Locked when beginTransaction has been called
      locked = true;        // Flag to show SPI access now locked
      SPI_BUSY_CHECK;       // Check send complete and clean out unused rx data
      CS_H;
      SET_BUS_READ_MODE;    // In case SPI has been configured for tx only
#if defined (SPI_HAS_TRANSACTION) && defined (SUPPORT_TRANSACTIONS) && !defined(TFT_PARALLEL_8_BIT) && !defined(RP2040_PIO_INTERFACE)
      spi.endTransaction();
#endif
    }
  }
}

/***************************************************************************************
** Function name:           begin_tft_read  (was called spi_begin_read)
** Description:             Start transaction for reads and select TFT
***************************************************************************************/
// Reads require a lower SPI clock rate than writes
inline void TFT_eeSPI::begin_tft_read(void){
  DMA_BUSY_CHECK; // Wait for any DMA transfer to complete before changing SPI settings
#if defined (SPI_HAS_TRANSACTION) && defined (SUPPORT_TRANSACTIONS) && !defined(TFT_PARALLEL_8_BIT) && !defined(RP2040_PIO_INTERFACE)
  if (locked) {
    locked = false;
    spi.beginTransaction(SPISettings(SPI_READ_FREQUENCY, MSBFIRST, TFT_SPI_MODE));
    CS_L;
  }
#else
  #if !defined(TFT_PARALLEL_8_BIT) && !defined(RP2040_PIO_INTERFACE)
    spi.setFrequency(SPI_READ_FREQUENCY);
  #endif
   CS_L;
#endif
  SET_BUS_READ_MODE;
}

/***************************************************************************************
** Function name:           end_tft_read (was called spi_end_read)
** Description:             End transaction for reads and deselect TFT
***************************************************************************************/
inline void TFT_eeSPI::end_tft_read(void){
#if defined (SPI_HAS_TRANSACTION) && defined (SUPPORT_TRANSACTIONS) && !defined(TFT_PARALLEL_8_BIT) && !defined(RP2040_PIO_INTERFACE)
  if(!inTransaction) {
    if (!locked) {
      locked = true;
      CS_H;
      spi.endTransaction();
    }
  }
#else
  #if !defined(TFT_PARALLEL_8_BIT) && !defined(RP2040_PIO_INTERFACE)
    spi.setFrequency(SPI_FREQUENCY);
  #endif
   if(!inTransaction) {CS_H;}
#endif
  SET_BUS_WRITE_MODE;
}


/***************************************************************************************
** Function name:           TFT_eeSPI
** Description:             Constructor , we must use hardware SPI pins
***************************************************************************************/
TFT_eeSPI::TFT_eeSPI()
{
  rotation  = 0;

  _swapBytes = false;   // Do not swap colour bytes by default

  locked = true;           // Transaction mutex lock flag to ensure begin/endTranaction pairing
  inTransaction = false;   // Flag to prevent multiple sequential functions to keep bus access open
  lockTransaction = false; // start/endWrite lock flag to allow sketch to keep SPI bus access open

  _booted   = true;     // Default attributes
#if defined (ESP32) && defined (CONFIG_SPIRAM_SUPPORT)
  if (psramFound()) _psram_enable = true; // Enable the use of PSRAM (if available)
  else
#endif
  _psram_enable = false;

  addr_row = 0xFFFF;  // drawPixel command length optimiser
  addr_col = 0xFFFF;  // drawPixel command length optimiser

// Legacy support for bit GPIO masks
  cspinmask = 0;
  dcpinmask = 0;
  wrpinmask = 0;
  sclkpinmask = 0;
}

/***************************************************************************************
** Function name:           initBus
** Description:             initialise the SPI or parallel bus
***************************************************************************************/
void TFT_eeSPI::initBus(void) {

#ifdef TFT_CS
  if (TFT_CS >= 0) {
    pinMode(TFT_CS, OUTPUT);
    digitalWrite(TFT_CS, HIGH); // Chip select high (inactive)
  }
#endif

// Configure chip select for touchscreen controller if present
#ifdef TOUCH_CS
  if (TOUCH_CS >= 0) {
    pinMode(TOUCH_CS, OUTPUT);
    digitalWrite(TOUCH_CS, HIGH); // Chip select high (inactive)
  }
#endif

// In parallel mode and with the RP2040 processor, the TFT_WR line is handled in the  PIO
#if defined (TFT_WR) && !defined (ARDUINO_ARCH_RP2040) && !defined (ARDUINO_ARCH_MBED)
  if (TFT_WR >= 0) {
    pinMode(TFT_WR, OUTPUT);
    digitalWrite(TFT_WR, HIGH); // Set write strobe high (inactive)
  }
#endif

#ifdef TFT_DC
  if (TFT_DC >= 0) {
    pinMode(TFT_DC, OUTPUT);
    digitalWrite(TFT_DC, HIGH); // Data/Command high = data mode
  }
#endif

#ifdef TFT_RST
  if (TFT_RST >= 0) {
    pinMode(TFT_RST, OUTPUT);
    digitalWrite(TFT_RST, HIGH); // Set high, do not share pin with another SPI device
  }
#endif

#if defined (TFT_PARALLEL_8_BIT)

  // Make sure read is high before we set the bus to output
  if (TFT_RD >= 0) {
    pinMode(TFT_RD, OUTPUT);
    digitalWrite(TFT_RD, HIGH);
  }

  #if  !defined (ARDUINO_ARCH_RP2040)  && !defined (ARDUINO_ARCH_MBED)// PIO manages pins
    // Set TFT data bus lines to output
    pinMode(TFT_D0, OUTPUT); digitalWrite(TFT_D0, HIGH);
    pinMode(TFT_D1, OUTPUT); digitalWrite(TFT_D1, HIGH);
    pinMode(TFT_D2, OUTPUT); digitalWrite(TFT_D2, HIGH);
    pinMode(TFT_D3, OUTPUT); digitalWrite(TFT_D3, HIGH);
    pinMode(TFT_D4, OUTPUT); digitalWrite(TFT_D4, HIGH);
    pinMode(TFT_D5, OUTPUT); digitalWrite(TFT_D5, HIGH);
    pinMode(TFT_D6, OUTPUT); digitalWrite(TFT_D6, HIGH);
    pinMode(TFT_D7, OUTPUT); digitalWrite(TFT_D7, HIGH);
  #endif

  PARALLEL_INIT_TFT_DATA_BUS;

#endif
}

/***************************************************************************************
** Function name:           begin
** Description:             Included for backwards compatibility
***************************************************************************************/
void TFT_eeSPI::begin(uint8_t tc, const uint8_t REV)
{
 init(tc, REV);
}

/***************************************************************************************
** Function name:           sendCmd
** Description:             tsdeeSPI
***************************************************************************************/
void TFT_eeSPI::sendCmd(const uint8_t cmd)
{
  writecommand(cmd);
}

/***************************************************************************************
** Function name:           sendCmdData
** Description:             tsdeeSPI
***************************************************************************************/
void TFT_eeSPI::sendCmdData(const uint8_t cmd, const uint8_t* data, const int16_t size)
{
  writecommand(cmd);
  for (int i = size; --i >= 0; ) {
    writedata(*data++);
  }
}

/***************************************************************************************
** Function name:           sendByte
** Description:             tsdeeSPI
***************************************************************************************/
void TFT_eeSPI::sendCmdByte(const uint8_t cmd, const uint8_t b)
{
  writecommand(cmd);
  writedata(b);
}


/***************************************************************************************
** Function name:           init (tc is tab colour for ST7735 displays only)
** Description:             Reset, then initialise the TFT display registers
***************************************************************************************/
void TFT_eeSPI::init(uint8_t tc, const uint8_t REV)
{
  if (_booted)
  {
    initBus();

#if !defined (ESP32) && !defined(TFT_PARALLEL_8_BIT) && !defined(ARDUINO_ARCH_RP2040) && !defined (ARDUINO_ARCH_MBED)
  // Legacy bitmasks for GPIO
  #if defined (TFT_CS) && (TFT_CS >= 0)
    cspinmask = (uint32_t) digitalPinToBitMask(TFT_CS);
  #endif

  #if defined (TFT_DC) && (TFT_DC >= 0)
    dcpinmask = (uint32_t) digitalPinToBitMask(TFT_DC);
  #endif

  #if defined (TFT_WR) && (TFT_WR >= 0)
    wrpinmask = (uint32_t) digitalPinToBitMask(TFT_WR);
  #endif

  #if defined (TFT_SCLK) && (TFT_SCLK >= 0)
    sclkpinmask = (uint32_t) digitalPinToBitMask(TFT_SCLK);
  #endif

  #if defined (TFT_SPI_OVERLAP) && defined (ARDUINO_ARCH_ESP8266)
    // Overlap mode SD0=MISO, SD1=MOSI, CLK=SCLK must use D3 as CS
    //    pins(int8_t sck, int8_t miso, int8_t mosi, int8_t ss);
    //spi.pins(        6,          7,           8,          0);
    spi.pins(6, 7, 8, 0);
  #endif

  spi.begin(); // This will set HMISO to input

#else
  #if !defined(TFT_PARALLEL_8_BIT) && !defined(RP2040_PIO_INTERFACE)
    #if defined (TFT_MOSI) && !defined (TFT_SPI_OVERLAP) && !defined(ARDUINO_ARCH_RP2040) && !defined (ARDUINO_ARCH_MBED)
      spi.begin(TFT_SCLK, TFT_MISO, TFT_MOSI, -1); // This will set MISO to input
    #else
      spi.begin(); // This will set MISO to input
    #endif
  #endif
#endif
    lockTransaction = false;
    inTransaction = false;
    locked = true;

    INIT_TFT_DATA_BUS;


#if defined (TFT_CS) && !defined(RP2040_PIO_INTERFACE)
  // Set to output once again in case MISO is used for CS
  if (TFT_CS >= 0) {
    pinMode(TFT_CS, OUTPUT);
    digitalWrite(TFT_CS, HIGH); // Chip select high (inactive)
  }
#elif defined (ARDUINO_ARCH_ESP8266) && !defined (TFT_PARALLEL_8_BIT) && !defined (RP2040_PIO_SPI)
  spi.setHwCs(1); // Use hardware SS toggling
#endif


  // Set to output once again in case MISO is used for DC
#if defined (TFT_DC) && !defined(RP2040_PIO_INTERFACE)
  if (TFT_DC >= 0) {
    pinMode(TFT_DC, OUTPUT);
    digitalWrite(TFT_DC, HIGH); // Data/Command high = data mode
  }
#endif

    _booted = false;
    end_tft_write();
  } // end of: if just _booted

  // Toggle RST low to reset
#ifdef TFT_RST
  #if !defined(RP2040_PIO_INTERFACE)
    // Set to output once again in case MISO is used for TFT_RST
    if (TFT_RST >= 0) {
      pinMode(TFT_RST, OUTPUT);
    }
  #endif
  if (TFT_RST >= 0) {
    writecommand(0x00); // Put SPI bus in known state for TFT with CS tied low
    digitalWrite(TFT_RST, HIGH);
    delay(5);
    digitalWrite(TFT_RST, LOW);
    delay(20);
    digitalWrite(TFT_RST, HIGH);
  }
  else writecommand(TFT_SWRST); // Software reset
#else
  writecommand(TFT_SWRST); // Software reset
#endif

  delay(150); // Wait for reset to complete

  begin_tft_write();

  tc = tc; // Suppress warning

#include "drivers_init.hh"

#ifdef TFT_INVERSION_ON
  writecommand(TFT_INVON);
#endif

#ifdef TFT_INVERSION_OFF
  writecommand(TFT_INVOFF);
#endif

  end_tft_write();

  setRotation(rotation, REV);

#if defined (TFT_BL) && defined (TFT_BACKLIGHT_ON)
  if (TFT_BL >= 0) {
    pinMode(TFT_BL, OUTPUT);
    digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);
  }
#else
  #if defined (TFT_BL) && defined (M5STACK)
    // Turn on the back-light LED
    if (TFT_BL >= 0) {
      pinMode(TFT_BL, OUTPUT);
      digitalWrite(TFT_BL, HIGH);
    }
  #endif
#endif
}


/***************************************************************************************
** Function name:           setRotation
** Description:             rotate the screen orientation m = 0-3 or 4-7 for BMP drawing
***************************************************************************************/
void TFT_eeSPI::setRotation(uint8_t m, const uint8_t REV)
{

  begin_tft_write();

  setRotationSizes(m);

#include "drivers_rotation.hh"

  delayMicroseconds(10);

  end_tft_write();

  addr_row = 0xFFFF;
  addr_col = 0xFFFF;

  // Reset the viewport to the whole screen
  resetViewport();
}


/***************************************************************************************
** Function name:           madctl to use instead of setSwapBytes
** Description:             tsdeeSPI
***************************************************************************************/
void TFT_eeSPI::setMADCTL(const uint8_t madctl)
{
  _madctl = madctl;
  sendCmdByte(TFT_MADCTL, _madctl);
}


/***************************************************************************************
** Function name:           getRotation
** Description:             Return the rotation value (as used by setRotation())
***************************************************************************************/
uint8_t TFT_eeSPI::getRotation(void)
{
  return rotation;
}


/***************************************************************************************
** Function name:           commandList, used for FLASH based lists only (e.g. ST7735)
** Description:             Get initialisation commands from FLASH and send to TFT
***************************************************************************************/
void TFT_eeSPI::commandList (const uint8_t *addr)
{
  uint8_t  numCommands;
  uint8_t  numArgs;
  uint8_t  ms;

  numCommands = pgm_read_byte(addr++);   // Number of commands to follow

  while (numCommands--)                  // For each command...
  {
    writecommand(pgm_read_byte(addr++)); // Read, issue command
    numArgs = pgm_read_byte(addr++);     // Number of args to follow
    ms = numArgs & TFT_INIT_DELAY;       // If high bit set, delay follows args
    numArgs &= ~TFT_INIT_DELAY;          // Mask out delay bit

    while (numArgs--)                    // For each argument...
    {
      writedata(pgm_read_byte(addr++));  // Read, issue argument
    }

    if (ms)
    {
      ms = pgm_read_byte(addr++);        // Read post-command delay time (ms)
      delay( (ms==255 ? 500 : ms) );
    }
  }

}


/***************************************************************************************
** Function name:           spiwrite
** Description:             Write 8 bits to SPI port (legacy support only)
***************************************************************************************/
void TFT_eeSPI::spiwrite(uint8_t c)
{
  begin_tft_write();
  tft_Write_8(c);
  end_tft_write();
}


/***************************************************************************************
** Function name:           writecommand
** Description:             Send an 8-bit command to the TFT
***************************************************************************************/
#ifndef RM68120_DRIVER
void TFT_eeSPI::writecommand(uint8_t c)
{
  begin_tft_write();

  DC_C;

  tft_Write_8(c);

  DC_D;

  end_tft_write();
}
#else
void TFT_eeSPI::writecommand(uint16_t c)
{
  begin_tft_write();

  DC_C;

  tft_Write_16(c);

  DC_D;

  end_tft_write();

}
void TFT_eeSPI::writeRegister8(uint16_t c, uint8_t d)
{
  begin_tft_write();

  DC_C;

  tft_Write_16(c);

  DC_D;

  tft_Write_8(d);

  end_tft_write();

}
void TFT_eSPI::writeRegister16(uint16_t c, uint16_t d)
{
  begin_tft_write();

  DC_C;

  tft_Write_16(c);

  DC_D;

  tft_Write_16(d);

  end_tft_write();

}

#endif

/***************************************************************************************
** Function name:           writedata
** Description:             Send a 8-bit data value to the TFT
***************************************************************************************/
void TFT_eeSPI::writedata(uint8_t d)
{
  begin_tft_write();

  DC_D;        // Play safe, but should already be in data mode

  tft_Write_8(d);

  CS_L;        // Allow more hold time for low VDI rail

  end_tft_write();
}


/***************************************************************************************
** Function name:           readcommand8
** Description:             Read a 8-bit data value from an indexed command register
***************************************************************************************/
uint8_t TFT_eeSPI::readcommand8(uint8_t cmd_function, uint8_t index)
{
  uint8_t reg = 0;
#if defined(TFT_PARALLEL_8_BIT) || defined(RP2040_PIO_INTERFACE)

  writecommand(cmd_function); // Sets DC and CS high

  busDir(GPIO_DIR_MASK, INPUT);

  CS_L;

  // Read nth parameter (assumes caller discards 1st parameter or points index to 2nd)
  while(index--) reg = readByte();

  busDir(GPIO_DIR_MASK, OUTPUT);

  CS_H;

#else // SPI interface
  // Tested with ILI9341 set to Interface II i.e. IM [3:0] = "1101"
  begin_tft_read();
  index = 0x10 + (index & 0x0F);

  DC_C; tft_Write_8(0xD9);
  DC_D; tft_Write_8(index);

  CS_H; // Some displays seem to need CS to be pulsed here, or is just a delay needed?
  CS_L;

  DC_C; tft_Write_8(cmd_function);
  DC_D;
  reg = tft_Read_8();

  end_tft_read();
#endif
  return reg;
}


/***************************************************************************************
** Function name:           readcommand16
** Description:             Read a 16-bit data value from an indexed command register
***************************************************************************************/
uint16_t TFT_eeSPI::readcommand16(uint8_t cmd_function, uint8_t index)
{
  uint32_t reg;

  reg  = (readcommand8(cmd_function, index + 0) <<  8);
  reg |= (readcommand8(cmd_function, index + 1) <<  0);

  return reg;
}


/***************************************************************************************
** Function name:           readcommand32
** Description:             Read a 32-bit data value from an indexed command register
***************************************************************************************/
uint32_t TFT_eeSPI::readcommand32(uint8_t cmd_function, uint8_t index)
{
  uint32_t reg;

  reg  = ((uint32_t)readcommand8(cmd_function, index + 0) << 24);
  reg |= ((uint32_t)readcommand8(cmd_function, index + 1) << 16);
  reg |= ((uint32_t)readcommand8(cmd_function, index + 2) <<  8);
  reg |= ((uint32_t)readcommand8(cmd_function, index + 3) <<  0);

  return reg;
}


/***************************************************************************************
** Function name:           read pixel (for SPI Interface II i.e. IM [3:0] = "1101")
** Description:             Read 565 pixel colours from a pixel
***************************************************************************************/
rgb_t TFT_eeSPI::readPixel(clip_t& clip, int32_t x, int32_t y)
{
  if (!clip.check_point(x, y)) return 0;

#if defined(TFT_PARALLEL_8_BIT) || defined(RP2040_PIO_INTERFACE)

  if (!inTransaction) { CS_L; } // CS_L can be multi-statement

  readAddrWindow(x, y, 1, 1);

  // Set masked pins D0- D7 to input
  busDir(GPIO_DIR_MASK, INPUT);

  #if  !defined (SSD1963_DRIVER)
  // Dummy read to throw away don't care value
    readByte();
  #endif

  // Fetch the 16-bit BRG pixel
  //uint16_t rgb = (readByte() << 8) | readByte();

  #if defined (ILI9341_DRIVER)  || defined(ILI9341_2_DRIVER) || defined (ILI9488_DRIVER) || defined (SSD1963_DRIVER)// Read 3 bytes

    // Read window pixel 24-bit RGB values and fill in LS bits
    uint32_t rgb = ((uint32_t)readByte() << 16) | (readByte() << 8) | readByte();

    if (!inTransaction) { CS_H; } // CS_H can be multi-statement

    // Set masked pins D0- D7 to output
    busDir(GPIO_DIR_MASK, OUTPUT);

    return rgb;

  #else // ILI9481 or ILI9486 16-bit read

    // Fetch the 16-bit BRG pixel
    uint16_t bgr = (readByte() << 8) | readByte();
    uint32_t rgb = color16to24(bgr);

    if (!inTransaction) { CS_H; } // CS_H can be multi-statement

    // Set masked pins D0- D7 to output
    busDir(GPIO_DIR_MASK, OUTPUT);

    #if defined (ILI9486_DRIVER) || defined (ST7796_DRIVER)
      return  rgb;
    #else
      // Swap Red and Blue (could check MADCTL setting to see if this is needed)
      return  ((rgb >> 16) & 0x0000FF) | ((rgb << 16) & 0xFF0000)| (rgb & 0x00FF00);
    #endif

  #endif

#else // Not TFT_PARALLEL_8_BIT

  // This function can get called during anti-aliased font rendering
  // so a transaction may be in progress
  bool wasInTransaction = inTransaction;
  if (inTransaction) { inTransaction= false; end_tft_write();}

  uint32_t color = 0;

  begin_tft_read(); // Sets CS low

  readAddrWindow(x, y, 1, 1);

  #ifdef TFT_SDA_READ
    begin_SDA_Read();
  #endif

  // Dummy read to throw away don't care value
  tft_Read_8();

  //#if !defined (ILI9488_DRIVER)

    #if defined (ST7796_DRIVER)
      // Read the 2 bytes
      color = ((tft_Read_8()) << 8) | (tft_Read_8());
    #elif defined (ST7735_DRIVER)
      // Read the 3 RGB bytes, colour is in LS 6 bits of the top 7 bits of each byte
      // as the TFT stores colours as 18 bits
      uint8_t r = tft_Read_8()<<1;
      uint8_t g = tft_Read_8()<<1;
      uint8_t b = tft_Read_8()<<1;
      color = RGB(r, g, b);
    #else
      // Read the 3 RGB bytes, colour is actually only in the top 6 bits of each byte
      // as the TFT stores colours as 18 bits
      uint8_t r = tft_Read_8();
      uint8_t g = tft_Read_8();
      uint8_t b = tft_Read_8();
      color = RGB(r, g, b);
    #endif

/*
  #else

    // The 6 colour bits are in MS 6 bits of each byte, but the ILI9488 needs an extra clock pulse
    // so bits appear shifted right 1 bit, so mask the middle 6 bits then shift 1 place left
    uint8_t r = (tft_Read_8()&0x7E)<<1;
    uint8_t g = (tft_Read_8()&0x7E)<<1;
    uint8_t b = (tft_Read_8()&0x7E)<<1;
    color = color565(r, g, b);

  #endif
*/
  CS_H;

  #ifdef TFT_SDA_READ
    end_SDA_Read();
  #endif

  end_tft_read();

  // Reinstate the transaction if one was in progress
  if(wasInTransaction) { begin_tft_write(); inTransaction = true; }

  return color;

#endif
}


/***************************************************************************************
** Function name:           read rectangle (for SPI Interface II i.e. IM [3:0] = "1101")
** Description:             Read 565 pixel colours from a defined area
***************************************************************************************/
void TFT_eeSPI::readRect(clip_t& clip, int32_t x0, int32_t y0, int32_t w, int32_t h, uint16_t *data)
{
  block_t z;
  if (!clip.check_block(z, x0, y0, w, h)) return;

#if defined(TFT_PARALLEL_8_BIT) || defined(RP2040_PIO_INTERFACE)

  CS_L;

  readAddrWindow(z.x, z.y, z.dw, z.dh);

  data += z.dx + z.dy * w;

  // Set masked pins D0- D7 to input
  busDir(GPIO_DIR_MASK, INPUT);

  #if defined (ILI9341_DRIVER)  || defined(ILI9341_2_DRIVER) || defined (ILI9488_DRIVER) // Read 3 bytes
    // Dummy read to throw away don't care value
    readByte();

    // Fetch the 24-bit RGB value
    while (z.dh--) {
      int32_t lw = z.dw;
      uint16_t* line = data;
      while (lw--) {
        // Assemble the RGB 16-bit colour
        uint16_t rgb = ((readByte() & 0xF8) << 8) | ((readByte() & 0xFC) << 3) | (readByte() >> 3);

        // Swapped byte order for compatibility with pushRect()
        *line++ = (rgb<<8) | (rgb>>8);
      }
      data += w;
    }

  #elif  defined (SSD1963_DRIVER)
    // Fetch the 18-bit BRG pixels
    while (z.dh--) {
      int32_t lw = z.dw;
      uint16_t* line = data;
      while (lw--) {
        uint16_t bgr = ((readByte() & 0xF8) >> 3);; // CS_L adds a small delay
        bgr |= ((readByte() & 0xFC) << 3);
        bgr |= (readByte() << 8);
        // Swap Red and Blue (could check MADCTL setting to see if this is needed)
        uint16_t rgb = (bgr>>11) | (bgr<<11) | (bgr & 0x7E0);
        // Swapped byte order for compatibility with pushRect()
        *line++ = (rgb<<8) | (rgb>>8);
      }
      data += w;
    }

  #else // ILI9481 reads as 16 bits
    // Dummy read to throw away don't care value
    readByte();

    // Fetch the 16-bit BRG pixels
    while (z.dh--) {
      int32_t lw = z.dw;
      uint16_t* line = data;
      while (lw--) {
      #if defined (ILI9486_DRIVER) || defined (ST7796_DRIVER)
        // Read the RGB 16-bit colour
        *line++ = readByte() | (readByte() << 8);
      #else
        // Read the BRG 16-bit colour
        uint16_t bgr = (readByte() << 8) | readByte();
        // Swap Red and Blue (could check MADCTL setting to see if this is needed)
        uint16_t rgb = (bgr>>11) | (bgr<<11) | (bgr & 0x7E0);
        // Swapped byte order for compatibility with pushRect()
        *line++ = (rgb<<8) | (rgb>>8);
      #endif
      }
      data += w;
    }
  #endif

  CS_H;

  // Set masked pins D0- D7 to output
  busDir(GPIO_DIR_MASK, OUTPUT);

#else // SPI interface

  // This function can get called after a begin_tft_write
  // so a transaction may be in progress
  bool wasInTransaction = inTransaction;
  if (inTransaction) { inTransaction= false; end_tft_write();}

  uint16_t color = 0;

  begin_tft_read();

  readAddrWindow(z.x, z.y, z.dw, z.dh);

  data += z.dx + z.dy * w;

  #ifdef TFT_SDA_READ
    begin_SDA_Read();
  #endif

  // Dummy read to throw away don't care value
  tft_Read_8();

  // Read window pixel 24-bit RGB values
  while (z.dh--) {
    int32_t lw = z.dw;
    uint16_t* line = data;
    while (lw--) {

  #if !defined (ILI9488_DRIVER)

    #if defined (ST7796_DRIVER)
      // Read the 2 bytes
      color = ((tft_Read_8()) << 8) | (tft_Read_8());
    #elif defined (ST7735_DRIVER)
      // Read the 3 RGB bytes, colour is in LS 6 bits of the top 7 bits of each byte
      // as the TFT stores colours as 18 bits
      uint8_t r = tft_Read_8()<<1;
      uint8_t g = tft_Read_8()<<1;
      uint8_t b = tft_Read_8()<<1;
      color = color565(r, g, b);
    #else
      // Read the 3 RGB bytes, colour is actually only in the top 6 bits of each byte
      // as the TFT stores colours as 18 bits
      uint8_t r = tft_Read_8();
      uint8_t g = tft_Read_8();
      uint8_t b = tft_Read_8();
      color = color565(r, g, b);
    #endif

  #else

      // The 6 colour bits are in MS 6 bits of each byte but we do not include the extra clock pulse
      // so we use a trick and mask the middle 6 bits of the byte, then only shift 1 place left
      uint8_t r = (tft_Read_8()&0x7E)<<1;
      uint8_t g = (tft_Read_8()&0x7E)<<1;
      uint8_t b = (tft_Read_8()&0x7E)<<1;
      color = color565(r, g, b);
  #endif

      // Swapped colour byte order for compatibility with pushRect()
      *line++ = color << 8 | color >> 8;
    }
    data += w;
  }

  //CS_H;

  #ifdef TFT_SDA_READ
    end_SDA_Read();
  #endif

  end_tft_read();

  // Reinstate the transaction if one was in progress
  if(wasInTransaction) { begin_tft_write(); inTransaction = true; }
#endif
}


/***************************************************************************************
** Function name:           setSwapBytes
** Description:             Used by 16-bit pushImage() to swap byte order in colours
***************************************************************************************/
void TFT_eeSPI::setSwapBytes(bool swap)
{
  _swapBytes = swap;
}


/***************************************************************************************
** Function name:           getSwapBytes
** Description:             Return the swap byte order for colours
***************************************************************************************/
bool TFT_eeSPI::getSwapBytes(void)
{
  return _swapBytes;
}


/***************************************************************************************
** Function name:           read rectangle (for SPI Interface II i.e. IM [3:0] = "1101")
** Description:             Read RGB pixel colours from a defined area
***************************************************************************************/
// If w and h are 1, then 1 pixel is read, *data array size must be 3 bytes per pixel
void  TFT_eeSPI::readRectRGB(clip_t& clip, int32_t x0, int32_t y0, int32_t w, int32_t h, uint8_t *data)
{
#if defined(TFT_PARALLEL_8_BIT) || defined(RP2040_PIO_INTERFACE)

  uint32_t len = w * h;
  uint8_t* buf565 = data + len;

  readRect(clip, x0, y0, w, h, (uint16_t*)buf565);

  while (len--) {
    uint16_t pixel565 = (*buf565++)<<8;
    pixel565 |= *buf565++;
    uint8_t red   = (pixel565 & 0xF800) >> 8; red   |= red   >> 5;
    uint8_t green = (pixel565 & 0x07E0) >> 3; green |= green >> 6;
    uint8_t blue  = (pixel565 & 0x001F) << 3; blue  |= blue  >> 5;
    *data++ = red;
    *data++ = green;
    *data++ = blue;
  }

#else  // Not TFT_PARALLEL_8_BIT

  begin_tft_read();

  readAddrWindow(x0, y0, w, h); // Sets CS low

  #ifdef TFT_SDA_READ
    begin_SDA_Read();
  #endif

  // Dummy read to throw away don't care value
  tft_Read_8();

  // Read window pixel 24-bit RGB values, buffer must be set in sketch to 3 * w * h
  uint32_t len = w * h;
  while (len--) {

  #if !defined (ILI9488_DRIVER)

    // Read the 3 RGB bytes, colour is actually only in the top 6 bits of each byte
    // as the TFT stores colours as 18 bits
    *data++ = tft_Read_8();
    *data++ = tft_Read_8();
    *data++ = tft_Read_8();

  #else

    // The 6 colour bits are in MS 6 bits of each byte, but the ILI9488 needs an extra clock pulse
    // so bits appear shifted right 1 bit, so mask the middle 6 bits then shift 1 place left
    *data++ = (tft_Read_8()&0x7E)<<1;
    *data++ = (tft_Read_8()&0x7E)<<1;
    *data++ = (tft_Read_8()&0x7E)<<1;

  #endif

  }

  CS_H;

  #ifdef TFT_SDA_READ
    end_SDA_Read();
  #endif

  end_tft_read();

#endif
}


/***************************************************************************************
** Function name:           drawCharDefault
** Description:             tsdeeSPI
***************************************************************************************/
void TFT_eeSPI::drawCharDefault(const uint8_t* font_offset, rgb_t color, rgb_t bg)
{
    uint8_t column[6];
    uint8_t mask = 0x1;

    for (int8_t i = 0; i < 5; i++ ) column[i] = pgm_read_byte(font_offset + i);
    column[5] = 0;

    for (int8_t j = 0; j < 8; j++) {
      for (int8_t k = 0; k < 5; k++ ) {
        if (column[k] & mask) {tft_Write_16(color);}
        else {tft_Write_16(bg);}
      }
      mask <<= 1;
      tft_Write_16(bg);
    }
}


/***************************************************************************************
** Function name:           setAddrWindow
** Description:             define an area to receive a stream of pixels
***************************************************************************************/
// Chip select is high at the end of this function
void TFT_eeSPI::setAddrWindow(int32_t x, int32_t y, int32_t w, int32_t h)
{
  begin_tft_write();

  setWindow(x, y, w, h);

  end_tft_write();
}


/***************************************************************************************
** Function name:           setWindow
** Description:             define an area to receive a stream of pixels
***************************************************************************************/
// Chip select stays low, call begin_tft_write first. Use setAddrWindow() from sketches
void TFT_eeSPI::setWindow(int32_t x0, int32_t y0, int32_t w, int32_t h)
{
  //begin_tft_write(); // Must be called before setWindow
  addr_row = 0xFFFF;
  addr_col = 0xFFFF;

  int32_t x1 = x0 + w - 1;
  int32_t y1 = y0 + h - 1;

#if defined (ILI9225_DRIVER)
  if (rotation & 0x01) { transpose(x0, y0); transpose(x1, y1); }
  SPI_BUSY_CHECK;
  DC_C; tft_Write_8(TFT_CASET1);
  DC_D; tft_Write_16(x0);
  DC_C; tft_Write_8(TFT_CASET2);
  DC_D; tft_Write_16(x1);

  DC_C; tft_Write_8(TFT_PASET1);
  DC_D; tft_Write_16(y0);
  DC_C; tft_Write_8(TFT_PASET2);
  DC_D; tft_Write_16(y1);

  DC_C; tft_Write_8(TFT_RAM_ADDR1);
  DC_D; tft_Write_16(x0);
  DC_C; tft_Write_8(TFT_RAM_ADDR2);
  DC_D; tft_Write_16(y0);

  // write to RAM
  DC_C; tft_Write_8(TFT_RAMWR);
  DC_D;
  // Temporary solution is to include the RP2040 code here
  #if (defined(ARDUINO_ARCH_RP2040)  || defined (ARDUINO_ARCH_MBED)) && !defined(RP2040_PIO_INTERFACE)
    // For ILI9225 and RP2040 the slower Arduino SPI transfer calls were used, so need to swap back to 16-bit mode
    while (spi_get_hw(SPI_X)->sr & SPI_SSPSR_BSY_BITS) {};
    hw_write_masked(&spi_get_hw(SPI_X)->cr0, (16 - 1) << SPI_SSPCR0_DSS_LSB, SPI_SSPCR0_DSS_BITS);
  #endif
#elif defined (SSD1351_DRIVER)
  if (rotation & 1) {
    transpose(x0, y0);
    transpose(x1, y1);
  }
  SPI_BUSY_CHECK;
  DC_C; tft_Write_8(TFT_CASET);
  DC_D; tft_Write_16(x1 | (x0 << 8));
  DC_C; tft_Write_8(TFT_PASET);
  DC_D; tft_Write_16(y1 | (y0 << 8));
  DC_C; tft_Write_8(TFT_RAMWR);
  DC_D;
#else
  #if defined (SSD1963_DRIVER)
    if ((rotation & 0x1) == 0) { transpose(x0, y0); transpose(x1, y1); }
  #endif

  #ifdef CGRAM_OFFSET
    x0+=colstart;
    x1+=colstart;
    y0+=rowstart;
    y1+=rowstart;
  #endif

  // Temporary solution is to include the RP2040 optimised code here
  #if (defined(ARDUINO_ARCH_RP2040)  || defined (ARDUINO_ARCH_MBED))
    #if !defined(RP2040_PIO_INTERFACE)
      // Use hardware SPI port, this code does not swap from 8 to 16-bit
      // to avoid the spi_set_format() call overhead
      while (spi_get_hw(SPI_X)->sr & SPI_SSPSR_BSY_BITS) {};
      DC_C;
      #if !defined (SPI_18BIT_DRIVER)
        #if  defined (RPI_DISPLAY_TYPE) // RPi TFT type always needs 16-bit transfers
          hw_write_masked(&spi_get_hw(SPI_X)->cr0, (16 - 1) << SPI_SSPCR0_DSS_LSB, SPI_SSPCR0_DSS_BITS);
        #else
          hw_write_masked(&spi_get_hw(SPI_X)->cr0, (8 - 1) << SPI_SSPCR0_DSS_LSB, SPI_SSPCR0_DSS_BITS);
        #endif
      #endif
      spi_get_hw(SPI_X)->dr = (uint32_t)TFT_CASET;

      while (spi_get_hw(SPI_X)->sr & SPI_SSPSR_BSY_BITS) {};
      DC_D;
      spi_get_hw(SPI_X)->dr = (uint32_t)x0>>8;
      spi_get_hw(SPI_X)->dr = (uint32_t)x0;
      spi_get_hw(SPI_X)->dr = (uint32_t)x1>>8;
      spi_get_hw(SPI_X)->dr = (uint32_t)x1;

      while (spi_get_hw(SPI_X)->sr & SPI_SSPSR_BSY_BITS) {};
      DC_C;
      spi_get_hw(SPI_X)->dr = (uint32_t)TFT_PASET;

      while (spi_get_hw(SPI_X)->sr & SPI_SSPSR_BSY_BITS) {};
      DC_D;
      spi_get_hw(SPI_X)->dr = (uint32_t)y0>>8;
      spi_get_hw(SPI_X)->dr = (uint32_t)y0;
      spi_get_hw(SPI_X)->dr = (uint32_t)y1>>8;
      spi_get_hw(SPI_X)->dr = (uint32_t)y1;

      while (spi_get_hw(SPI_X)->sr & SPI_SSPSR_BSY_BITS) {};
      DC_C;
      spi_get_hw(SPI_X)->dr = (uint32_t)TFT_RAMWR;

      while (spi_get_hw(SPI_X)->sr & SPI_SSPSR_BSY_BITS) {};
      #if !defined (SPI_18BIT_DRIVER)
        hw_write_masked(&spi_get_hw(SPI_X)->cr0, (16 - 1) << SPI_SSPCR0_DSS_LSB, SPI_SSPCR0_DSS_BITS);
      #endif
      DC_D;
    #elif defined (RM68120_DRIVER)
      DC_C; tft_Write_16(TFT_CASET+0); DC_D; tft_Write_16(x0 >> 8);
      DC_C; tft_Write_16(TFT_CASET+1); DC_D; tft_Write_16(x0 & 0xFF);
      DC_C; tft_Write_16(TFT_CASET+2); DC_D; tft_Write_16(x1 >> 8);
      DC_C; tft_Write_16(TFT_CASET+3); DC_D; tft_Write_16(x1 & 0xFF);
      DC_C; tft_Write_16(TFT_PASET+0); DC_D; tft_Write_16(y0 >> 8);
      DC_C; tft_Write_16(TFT_PASET+1); DC_D; tft_Write_16(y0 & 0xFF);
      DC_C; tft_Write_16(TFT_PASET+2); DC_D; tft_Write_16(y1 >> 8);
      DC_C; tft_Write_16(TFT_PASET+3); DC_D; tft_Write_16(y1 & 0xFF);

      DC_C; tft_Write_16(TFT_RAMWR);
      DC_D;
    #else
      // This is for the RP2040 and PIO interface (SPI or parallel)
      WAIT_FOR_STALL;
      tft_pio->sm[pio_sm].instr = pio_instr_addr;

      TX_FIFO = TFT_CASET;
      TX_FIFO = (x0<<16) | x1;
      TX_FIFO = TFT_PASET;
      TX_FIFO = (y0<<16) | y1;
      TX_FIFO = TFT_RAMWR;
    #endif
  #else
    SPI_BUSY_CHECK;
    DC_C; tft_Write_8(TFT_CASET);
    DC_D; tft_Write_32C(x0, x1);
    DC_C; tft_Write_8(TFT_PASET);
    DC_D; tft_Write_32C(y0, y1);
    DC_C; tft_Write_8(TFT_RAMWR);
    DC_D;
  #endif // RP2040 SPI
#endif
  //end_tft_write(); // Must be called after setWindow
}

/***************************************************************************************
** Function name:           readAddrWindow
** Description:             define an area to read a stream of pixels
***************************************************************************************/
void TFT_eeSPI::readAddrWindow(int32_t xs, int32_t ys, int32_t w, int32_t h)
{
  //begin_tft_write(); // Must be called before readAddrWindow or CS set low

  int32_t xe = xs + w - 1;
  int32_t ye = ys + h - 1;

  addr_col = 0xFFFF;
  addr_row = 0xFFFF;

#if defined (SSD1963_DRIVER)
  if ((rotation & 0x1) == 0) { transpose(xs, ys); transpose(xe, ye); }
#endif

#ifdef CGRAM_OFFSET
  xs += colstart;
  xe += colstart;
  ys += rowstart;
  ye += rowstart;
#endif

  // Temporary solution is to include the RP2040 optimised code here
#if (defined(ARDUINO_ARCH_RP2040)  || defined (ARDUINO_ARCH_MBED)) && !defined(RP2040_PIO_INTERFACE)
  // Use hardware SPI port, this code does not swap from 8 to 16-bit
  // to avoid the spi_set_format() call overhead
  while (spi_get_hw(SPI_X)->sr & SPI_SSPSR_BSY_BITS) {};
  DC_C;
  hw_write_masked(&spi_get_hw(SPI_X)->cr0, (8 - 1) << SPI_SSPCR0_DSS_LSB, SPI_SSPCR0_DSS_BITS);
  spi_get_hw(SPI_X)->dr = (uint32_t)TFT_CASET;

  while (spi_get_hw(SPI_X)->sr & SPI_SSPSR_BSY_BITS) {};
  DC_D;
  spi_get_hw(SPI_X)->dr = (uint32_t)xs>>8;
  spi_get_hw(SPI_X)->dr = (uint32_t)xs;
  spi_get_hw(SPI_X)->dr = (uint32_t)xe>>8;
  spi_get_hw(SPI_X)->dr = (uint32_t)xe;

  while (spi_get_hw(SPI_X)->sr & SPI_SSPSR_BSY_BITS) {};
  DC_C;
  spi_get_hw(SPI_X)->dr = (uint32_t)TFT_PASET;

  while (spi_get_hw(SPI_X)->sr & SPI_SSPSR_BSY_BITS) {};
  DC_D;
  spi_get_hw(SPI_X)->dr = (uint32_t)ys>>8;
  spi_get_hw(SPI_X)->dr = (uint32_t)ys;
  spi_get_hw(SPI_X)->dr = (uint32_t)ye>>8;
  spi_get_hw(SPI_X)->dr = (uint32_t)ye;

  while (spi_get_hw(SPI_X)->sr & SPI_SSPSR_BSY_BITS) {};
  DC_C;
  spi_get_hw(SPI_X)->dr = (uint32_t)TFT_RAMRD;

  while (spi_get_hw(SPI_X)->sr & SPI_SSPSR_BSY_BITS) {};
  DC_D;

  // Flush the rx buffer and reset overflow flag
  while (spi_is_readable(SPI_X)) (void)spi_get_hw(SPI_X)->dr;
  spi_get_hw(SPI_X)->icr = SPI_SSPICR_RORIC_BITS;

#else
  // Column addr set
  DC_C; tft_Write_8(TFT_CASET);
  DC_D; tft_Write_32C(xs, xe);

  // Row addr set
  DC_C; tft_Write_8(TFT_PASET);
  DC_D; tft_Write_32C(ys, ye);

  // Read CGRAM command
  DC_C; tft_Write_8(TFT_RAMRD);

  DC_D;
#endif // RP2040 SPI

  //end_tft_write(); // Must be called after readAddrWindow or CS set high
}


/***************************************************************************************
** Function name:           drawPixel
** Description:             push a single pixel at an arbitrary position
***************************************************************************************/
void TFT_eeSPI::drawPixel(clip_t& clip, int32_t x, int32_t y, rgb_t color)
{
  if (!clip.check_point(x, y)) return;

#ifdef CGRAM_OFFSET
  x += colstart;
  y += rowstart;
#endif

#if (defined (MULTI_TFT_SUPPORT) || defined (GC9A01_DRIVER)) && !defined (ILI9225_DRIVER)
  addr_row = 0xFFFF;
  addr_col = 0xFFFF;
#endif

  begin_tft_write();

#if defined (ILI9225_DRIVER)
  if (rotation & 0x01) { transpose(x, y); }
  SPI_BUSY_CHECK;

  // Set window to full screen to optimise sequential pixel rendering
  if (addr_row != 0x9225) {
    addr_row = 0x9225; // addr_row used for flag
    DC_C; tft_Write_8(TFT_CASET1);
    DC_D; tft_Write_16(0);
    DC_C; tft_Write_8(TFT_CASET2);
    DC_D; tft_Write_16(175);

    DC_C; tft_Write_8(TFT_PASET1);
    DC_D; tft_Write_16(0);
    DC_C; tft_Write_8(TFT_PASET2);
    DC_D; tft_Write_16(219);
  }

  // Define pixel coordinate
  DC_C; tft_Write_8(TFT_RAM_ADDR1);
  DC_D; tft_Write_16(x);
  DC_C; tft_Write_8(TFT_RAM_ADDR2);
  DC_D; tft_Write_16(y);

  // write to RAM
  DC_C; tft_Write_8(TFT_RAMWR);
  #if defined(TFT_PARALLEL_8_BIT) || defined(TFT_PARALLEL_16_BIT) || !defined(ESP32)
    DC_D; tft_Write_16(color);
  #else
    DC_D; tft_Write_16N(color);
  #endif

// Temporary solution is to include the RP2040 optimised code here
#elif (defined (ARDUINO_ARCH_RP2040) || defined (ARDUINO_ARCH_MBED)) && !defined (SSD1351_DRIVER)

  #if defined (SSD1963_DRIVER)
    if ((rotation & 0x1) == 0) { transpose(x, y); }
  #endif

  #if !defined(RP2040_PIO_INTERFACE)
    while (spi_get_hw(SPI_X)->sr & SPI_SSPSR_BSY_BITS) {};

    #if  defined (RPI_DISPLAY_TYPE) // RPi TFT type always needs 16-bit transfers
      hw_write_masked(&spi_get_hw(SPI_X)->cr0, (16 - 1) << SPI_SSPCR0_DSS_LSB, SPI_SSPCR0_DSS_BITS);
    #else
      hw_write_masked(&spi_get_hw(SPI_X)->cr0, (8 - 1) << SPI_SSPCR0_DSS_LSB, SPI_SSPCR0_DSS_BITS);
    #endif

    if (addr_col != x) {
      DC_C;
      spi_get_hw(SPI_X)->dr = (uint32_t)TFT_CASET;
      while (spi_get_hw(SPI_X)->sr & SPI_SSPSR_BSY_BITS){};
      DC_D;
      spi_get_hw(SPI_X)->dr = (uint32_t)x>>8;
      spi_get_hw(SPI_X)->dr = (uint32_t)x;
      spi_get_hw(SPI_X)->dr = (uint32_t)x>>8;
      spi_get_hw(SPI_X)->dr = (uint32_t)x;
      addr_col = x;
      while (spi_get_hw(SPI_X)->sr & SPI_SSPSR_BSY_BITS) {};
    }

    if (addr_row != y) {
      DC_C;
      spi_get_hw(SPI_X)->dr = (uint32_t)TFT_PASET;
      while (spi_get_hw(SPI_X)->sr & SPI_SSPSR_BSY_BITS) {};
      DC_D;
      spi_get_hw(SPI_X)->dr = (uint32_t)y>>8;
      spi_get_hw(SPI_X)->dr = (uint32_t)y;
      spi_get_hw(SPI_X)->dr = (uint32_t)y>>8;
      spi_get_hw(SPI_X)->dr = (uint32_t)y;
      addr_row = y;
      while (spi_get_hw(SPI_X)->sr & SPI_SSPSR_BSY_BITS) {};
    }

    DC_C;
    spi_get_hw(SPI_X)->dr = (uint32_t)TFT_RAMWR;

    #if defined (SPI_18BIT_DRIVER) // SPI 18-bit colour
      uint8_t r = (color & 0xF800)>>8;
      uint8_t g = (color & 0x07E0)>>3;
      uint8_t b = (color & 0x001F)<<3;
      while (spi_get_hw(SPI_X)->sr & SPI_SSPSR_BSY_BITS) {};
      DC_D;
      tft_Write_8N(r); tft_Write_8N(g); tft_Write_8N(b);
    #else
      while (spi_get_hw(SPI_X)->sr & SPI_SSPSR_BSY_BITS) {};
      DC_D;
      #if  defined (RPI_DISPLAY_TYPE) // RPi TFT type always needs 16-bit transfers
        spi_get_hw(SPI_X)->dr = (uint32_t)color;
      #else
        spi_get_hw(SPI_X)->dr = (uint32_t)color>>8;
        spi_get_hw(SPI_X)->dr = (uint32_t)color;
      #endif
    #endif
    while (spi_get_hw(SPI_X)->sr & SPI_SSPSR_BSY_BITS) {};
  #elif defined (RM68120_DRIVER)
    if (addr_col != x) {
      DC_C; tft_Write_16(TFT_CASET+0); DC_D; tft_Write_16(x >> 8);
      DC_C; tft_Write_16(TFT_CASET+1); DC_D; tft_Write_16(x & 0xFF);
      DC_C; tft_Write_16(TFT_CASET+2); DC_D; tft_Write_16(x >> 8);
      DC_C; tft_Write_16(TFT_CASET+3); DC_D; tft_Write_16(x & 0xFF);
      addr_col = x;
    }
    if (addr_row != y) {
      DC_C; tft_Write_16(TFT_PASET+0); DC_D; tft_Write_16(y >> 8);
      DC_C; tft_Write_16(TFT_PASET+1); DC_D; tft_Write_16(y & 0xFF);
      DC_C; tft_Write_16(TFT_PASET+2); DC_D; tft_Write_16(y >> 8);
      DC_C; tft_Write_16(TFT_PASET+3); DC_D; tft_Write_16(y & 0xFF);
      addr_row = y;
    }
    DC_C; tft_Write_16(TFT_RAMWR); DC_D;

    TX_FIFO = color;
  #else
    // This is for the RP2040 and PIO interface (SPI or parallel)
    WAIT_FOR_STALL;
    tft_pio->sm[pio_sm].instr = pio_instr_addr;
    TX_FIFO = TFT_CASET;
    TX_FIFO = (x<<16) | x;
    TX_FIFO = TFT_PASET;
    TX_FIFO = (y<<16) | y;
    TX_FIFO = TFT_RAMWR;
    //DC set high by PIO
    #if  defined (SPI_18BIT_DRIVER) || (defined (SSD1963_DRIVER) && defined (TFT_PARALLEL_8_BIT))
      TX_FIFO = ((color & 0xF800)<<8) | ((color & 0x07E0)<<5) | ((color & 0x001F)<<3);
    #else
      TX_FIFO = color;
    #endif

  #endif

#else

  #if defined (SSD1963_DRIVER)
    if ((rotation & 0x1) == 0) { transpose(x, y); }
  #endif

    SPI_BUSY_CHECK;

  #if defined (SSD1351_DRIVER)
    if (rotation & 0x1) { transpose(x, y); }
    // No need to send x if it has not changed (speeds things up)
    if (addr_col != x) {
      DC_C; tft_Write_8(TFT_CASET);
      DC_D; tft_Write_16(x | (x << 8));
      addr_col = x;
    }

    // No need to send y if it has not changed (speeds things up)
    if (addr_row != y) {
      DC_C; tft_Write_8(TFT_PASET);
      DC_D; tft_Write_16(y | (y << 8));
      addr_row = y;
    }
  #else
    // No need to send x if it has not changed (speeds things up)
    if (addr_col != x) {
      DC_C; tft_Write_8(TFT_CASET);
      DC_D; tft_Write_32D(x);
      addr_col = x;
    }

    // No need to send y if it has not changed (speeds things up)
    if (addr_row != y) {
      DC_C; tft_Write_8(TFT_PASET);
      DC_D; tft_Write_32D(y);
      addr_row = y;
    }
  #endif

  DC_C; tft_Write_8(TFT_RAMWR);

  #if defined(TFT_PARALLEL_8_BIT) || defined(TFT_PARALLEL_16_BIT) || !defined(ESP32)
    DC_D; tft_Write_16(color);
  #else
    DC_D; tft_Write_16N(color);
  #endif
#endif

  end_tft_write();
}

/***************************************************************************************
** Function name:           pushColor
** Description:             push a single pixel
***************************************************************************************/
void TFT_eeSPI::pushColor(rgb_t color)
{
  begin_tft_write();

  SPI_BUSY_CHECK;
#ifdef COLOR_565
  tft_Write_16N(mdt_color(color));
#else
#endif

  end_tft_write();
}


/***************************************************************************************
** Function name:           pushColor
** Description:             push a single colour to "len" pixels
***************************************************************************************/
void TFT_eeSPI::pushColor(rgb_t color, uint32_t len)
{
  begin_tft_write();

#ifdef COLOR_565
  pushBlock16(mdt_color(color), len);
#else
#endif

  end_tft_write();
}

/***************************************************************************************
** Function name:           startWrite
** Description:             begin transaction with CS low, MUST later call endWrite
***************************************************************************************/
void TFT_eeSPI::startWrite(void)
{
  begin_tft_write();
  lockTransaction = true; // Lock transaction for all sequentially run sketch functions
  inTransaction = true;
}

/***************************************************************************************
** Function name:           endWrite
** Description:             end transaction with CS high
***************************************************************************************/
void TFT_eeSPI::endWrite(void)
{
  lockTransaction = false; // Release sketch induced transaction lock
  inTransaction = false;
  DMA_BUSY_CHECK;          // Safety check - user code should have checked this!
  end_tft_write();         // Release SPI bus
}

/***************************************************************************************
** Function name:           writeColor (use startWrite() and endWrite() before & after)
** Description:             raw write of "len" pixels avoiding transaction check
***************************************************************************************/
void TFT_eeSPI::writeColor(rgb_t color, uint32_t len)
{
#ifdef COLOR_565
  pushBlock16(color, len);
#else
#endif
}

/***************************************************************************************
** Function name:           pushColors
** Description:             push an array of pixels for 16-bit raw image drawing
***************************************************************************************/
// Assumed that setAddrWindow() has previously been called
// len is number of bytes, not pixels
void TFT_eeSPI::pushColors(uint8_t *data, uint32_t len)
{
  begin_tft_write();

#ifdef COLOR_565
  pushPixels16(data, len>>1);
#else
#endif

  end_tft_write();
}


/***************************************************************************************
** Function name:           pushColors
** Description:             push an array of pixels, for image drawing
***************************************************************************************/
void TFT_eeSPI::pushColors(uint16_t *data, uint32_t len)
{
  begin_tft_write();

#ifdef COLOR_565
  pushPixels16(data, len);
#else
#endif

  end_tft_write();
}


/***************************************************************************************
** Function name:           color565
** Description:             convert three 8-bit RGB levels to a 16-bit colour value
***************************************************************************************/
uint16_t TFT_eeSPI::color565(uint8_t r, uint8_t g, uint8_t b)
{
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}




/***************************************************************************************
** Function name:           invertDisplay
** Description:             invert the display colours i = 1 invert, i = 0 normal
***************************************************************************************/
void TFT_eeSPI::invertDisplay(bool i)
{
  begin_tft_write();
  // Send the command twice as otherwise it does not always work!
  writecommand(i ? TFT_INVON : TFT_INVOFF);
  writecommand(i ? TFT_INVON : TFT_INVOFF);
  end_tft_write();
}



/***************************************************************************************
** Function name:           drawCharFaster
** Description:             tsdeeSPI
***************************************************************************************/
void TFT_eeSPI::drawCharFont2Faster(uint16_t width, uint16_t height, rgb_t textcolor, rgb_t textbgcolor, uint32_t flash_address)
{
    int32_t w = width;
    w = w + 6; // Should be + 7 but we need to compensate for width increment
    w = w / 8;

    uint8_t mask;
      for (int32_t i = 0; i < height; i++) {
        int32_t pX = width;
        for (int32_t k = 0; k < w; k++) {
          uint8_t line = pgm_read_byte((uint8_t *) (flash_address + w * i + k) );
          mask = 0x80;
          while (mask && pX) {
            if (line & mask) {tft_Write_16(textcolor);}
            else {tft_Write_16(textbgcolor);}
            pX--;
            mask = mask >> 1;
          }
        }
        if (pX) {tft_Write_16(textbgcolor);}
      }
}


/***************************************************************************************
** Function name:           drawCharRLEfont
** Description:             tsdeeSPI
***************************************************************************************/
void TFT_eeSPI::drawCharRLEfont(int32_t xd, int32_t yd, int32_t pY, uint16_t width, uint16_t height, int16_t textsize, rgb_t textcolor, uint32_t flash_address)
{
    int32_t w = width;
    w *= height; // Now w is total number of pixels in the character

      int32_t px = 0, py = pY; // To hold character block start and end column and row values
      int32_t pc = 0; // Pixel count
      uint8_t np = textsize * textsize; // Number of pixels in a drawn pixel

      // 16-bit pixel count so maximum font size is equivalent to 180x180 pixels in area
      // w is total number of pixels to plot to fill character block
      while (pc < w) {
        uint8_t line = pgm_read_byte((uint8_t *)flash_address);
        flash_address++;
        if (line & 0x80) {
          line &= 0x7F;
          line++;
          if (textsize > 1) {
            px = xd + textsize * (pc % width); // Keep these px and py calculations outside the loop as they are slow
            py = yd + textsize * (pc / width);
          }
          else {
            px = xd + pc % width; // Keep these px and py calculations outside the loop as they are slow
            py = yd + pc / width;
          }
          while (line--) { // In this case the while(line--) is faster
            pc++; // This is faster than putting pc+=line before while()?
            setWindow(px, py, textsize, textsize);

            if (textsize > 1) {
              int16_t j = np;
              while (j--) {tft_Write_16(textcolor);}
            }
            else {tft_Write_16(textcolor);}
            px += textsize;

            if (px >= (xd + width * textsize)) {
              px = xd;
              py += textsize;
            }
          }
        }
        else {
          line++;
          pc += line;
        }
      }
}


/***************************************************************************************
** Function name:           getSPIinstance
** Description:             Get the instance of the SPI class
***************************************************************************************/
#if !defined (TFT_PARALLEL_8_BIT) && !defined (RP2040_PIO_INTERFACE)
SPIClass& TFT_eeSPI::getSPIinstance(void)
{
  return spi;
}
#endif

////////////////////////////////////////////////////////////////////////////////////////
#ifdef TOUCH_CS
  #include "Extensions/Touch.cpp"
#endif

////////////////////////////////////////////////////////////////////////////////////////


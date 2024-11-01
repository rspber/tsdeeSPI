
#pragma once

// Change the width and height if required (defined in portrait mode)
// or use the constructor to over-ride defaults
#if defined (SSD1963_480_DRIVER)
  #define TFT_WIDTH  272
  #define TFT_HEIGHT 480
#elif defined (SSD1963_800_DRIVER)
  #define TFT_WIDTH  480
  #define TFT_HEIGHT 800
#elif defined (SSD1963_800ALT_DRIVER)
  #define TFT_WIDTH  480
  #define TFT_HEIGHT 800
#elif defined (SSD1963_800BD_DRIVER)
  #define TFT_WIDTH  480
  #define TFT_HEIGHT 800
#endif

//Set driver type common to all initialisation options
#ifndef SSD1963_DRIVER
  #define SSD1963_DRIVER
#endif

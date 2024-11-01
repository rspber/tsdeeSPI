
#if defined (RM68120_DRIVER)
     #include "TFT_Drivers/TFT_RM68120.h"
     #define  TFT_DRIVER 0x6812
#elif defined (SSD1351_DRIVER)
     #include "TFT_Drivers/TFT_SSD1351.h"
     #define  TFT_DRIVER 0x1351
#elif defined (ILI9225_DRIVER)
     #include "TFT_Drivers/TFT_ILI9225.h"
     #define  TFT_DRIVER 0x9225
#else

  #include <TFT_Drivers/TFT_Common.h>

// Load the right driver definition - do not tinker here !
#if   defined (ILI9341_DRIVER) || defined(ILI9341_2_DRIVER) || defined (ILI9342_DRIVER)
     #include <TFT_Drivers/TFT_ILI9341.h>
     #define  TFT_DRIVER 0x9341
#elif defined (ST7735_DRIVER)
     #include <TFT_Drivers/TFT_ST7735.h>
     #define  TFT_DRIVER 0x7735
#elif defined (ILI9163_DRIVER)
     #include <TFT_Drivers/TFT_ILI9163.h>
     #define  TFT_DRIVER 0x9163
#elif defined (S6D02A1_DRIVER)
     #include <TFT_Drivers/TFT_S6D02A1.h>
     #define  TFT_DRIVER 0x6D02
#elif defined (ST7796_DRIVER)
      #include "TFT_Drivers/TFT_ST7796.h"
      #define  TFT_DRIVER 0x7796
#elif defined (ILI9486_DRIVER)
     #include <TFT_Drivers/TFT_ILI9486.h>
     #define  TFT_DRIVER 0x9486
#elif defined (ILI9481_DRIVER)
     #include <TFT_Drivers/TFT_ILI9481.h>
     #define  TFT_DRIVER 0x9481
#elif defined (ILI9488_DRIVER)
     #include <TFT_Drivers/TFT_ILI9488.h>
     #define  TFT_DRIVER 0x9488
#elif defined (HX8357D_DRIVER)
     #include "TFT_Drivers/TFT_HX8357D.h"
     #define  TFT_DRIVER 0x8357
#elif defined (EPD_DRIVER)
     #include "TFT_Drivers/TFT_EPD.h"
     #define  TFT_DRIVER 0xE9D
#elif defined (ST7789_DRIVER)
     #include "TFT_Drivers/TFT_ST7789.h"
     #define  TFT_DRIVER 0x7789
#elif defined (R61581_DRIVER)
     #include "TFT_Drivers/TFT_R61581.h"
     #define  TFT_DRIVER 0x6158
#elif defined (RM68140_DRIVER)
     #include "TFT_Drivers/TFT_RM68140.h"
     #define  TFT_DRIVER 0x6814
#elif defined (SSD1963_DRIVER)
     #include "TFT_Drivers/TFT_SSD1963.h"
     #define  TFT_DRIVER 0x1963
#elif defined (GC9A01_DRIVER)
     #include "TFT_Drivers/TFT_GC9A01.h"
     #define  TFT_DRIVER 0x9A01
#elif defined (HX8357B_DRIVER)
     #include "TFT_Drivers/TFT_HX8357B.h"
     #define  TFT_DRIVER 0x835B
#elif defined (HX8357C_DRIVER)
     #include "TFT_Drivers/TFT_HX8357C.h"
     #define  TFT_DRIVER 0x835C

                              // <<<<<<<<<<<<<<<<<<<<<<<< ADD NEW DRIVER HERE
                              // XYZZY_init.h and XYZZY_rotation.h must also be added in TFT_eSPI.cpp
#elif defined (XYZZY_DRIVER)
     #include "TFT_Drivers/TFT_XYZZY.h"
     #define  TFT_DRIVER 0x0000
#else
     #define  TFT_DRIVER 0x0000
#endif

#endif


  // This loads the driver specific rotation code  <<<<<<<<<<<<<<<<<<<<< ADD NEW DRIVERS TO THE LIST HERE <<<<<<<<<<<<<<<<<<<<<<<
#if   defined (ILI9341_DRIVER) || defined(ILI9341_2_DRIVER) || defined (ILI9342_DRIVER)
    #include "TFT_Drivers/ILI9xxx_Rotation.hh"

#elif defined (ST7735_DRIVER)
    #include "TFT_Drivers/ST7735_Rotation.hh"

#elif defined (ILI9163_DRIVER)
    #include "TFT_Drivers/ILI9163_Rotation.hh"

#elif defined (S6D02A1_DRIVER)
    #include "TFT_Drivers/S6D02A1_Rotation.hh"

#elif defined (ST7796_DRIVER)
    #include "TFT_Drivers/ST7796_Rotation.hh"

#elif defined (ILI9486_DRIVER)
    #include "TFT_Drivers/ILI9486_Rotation.hh"

#elif defined (ILI9481_DRIVER)
    #include "TFT_Drivers/ILI9x81_Rotation.hh"

#elif defined (ILI9488_DRIVER)
    #include "TFT_Drivers/ILI9488_Rotation.hh"

#elif defined (HX8357D_DRIVER)
    #include "TFT_Drivers/HX8357D_Rotation.hh"

#elif defined (ST7789_DRIVER)
    #include "TFT_Drivers/ILI9xxx_Rotation.hh"

#elif defined (R61581_DRIVER)
    #include "TFT_Drivers/R61581_Rotation.hh"

#elif defined (RM68140_DRIVER)
	#include "TFT_Drivers/RM68140_Rotation.hh"

#elif defined (SSD1351_DRIVER)
    #include "TFT_Drivers/SSD1351_Rotation.hh"

#elif defined (SSD1963_DRIVER)
    #include "TFT_Drivers/SSD1963_Rotation.hh"

#elif defined (GC9A01_DRIVER)
     #include "TFT_Drivers/GC9A01_Rotation.hh"

#elif defined (ILI9225_DRIVER)
     #include "TFT_Drivers/ILI9225_Rotation.hh"

#elif defined (RM68120_DRIVER)
     #include "TFT_Drivers/RM68120_Rotation.hh"

#elif defined (HX8357B_DRIVER)
    #include "TFT_Drivers/HX8357B_Rotation.hh"

#elif defined (HX8357C_DRIVER)
    #include "TFT_Drivers/HX8357B_Rotation.hh"

#endif

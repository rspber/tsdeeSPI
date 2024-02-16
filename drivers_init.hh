
  // This loads the driver specific initialisation code  <<<<<<<<<<<<<<<<<<<<< ADD NEW DRIVERS TO THE LIST HERE <<<<<<<<<<<<<<<<<<<<<<<
#if   defined (ILI9341_DRIVER) || defined(ILI9341_2_DRIVER) || defined (ILI9342_DRIVER)
    #include "TFT_Drivers/ILI9341_Init.h"

#elif defined (ST7735_DRIVER)
    tabcolor = tc;
    #include "TFT_Drivers/ST7735_Init.h"

#elif defined (ILI9163_DRIVER)
    #include "TFT_Drivers/ILI9163_Init.h"

#elif defined (S6D02A1_DRIVER)
    #include "TFT_Drivers/S6D02A1_Init.h"

#elif defined (ST7796_DRIVER)
    #include "TFT_Drivers/ST7796_Init.h"

#elif defined (ILI9486_DRIVER)
    #include "TFT_Drivers/ILI9486_Init.h"

#elif defined (ILI9481_DRIVER)
    #include "TFT_Drivers/ILI9481_Init.h"

#elif defined (ILI9488_DRIVER)
    #include "TFT_Drivers/ILI9488_Init.h"

#elif defined (HX8357D_DRIVER)
    #include "TFT_Drivers/HX8357D_Init.h"

#elif defined (ST7789_DRIVER)
    #include "TFT_Drivers/ST7789_Init.h"

#elif defined (R61581_DRIVER)
    #include "TFT_Drivers/R61581_Init.h"

#elif defined (RM68140_DRIVER)
	#include "TFT_Drivers/RM68140_Init.h"

#elif defined (ST7789_2_DRIVER)
    #include "TFT_Drivers/ST7789_2_Init.h"

#elif defined (SSD1351_DRIVER)
    #include "TFT_Drivers/SSD1351_Init.h"

#elif defined (SSD1963_DRIVER)
    #include "TFT_Drivers/SSD1963_Init.h"

#elif defined (GC9A01_DRIVER)
     #include "TFT_Drivers/GC9A01_Init.h"

#elif defined (ILI9225_DRIVER)
     #include "TFT_Drivers/ILI9225_Init.h"

#elif defined (RM68120_DRIVER)
     #include "TFT_Drivers/RM68120_Init.h"

#elif defined (HX8357B_DRIVER)
    #include "TFT_Drivers/HX8357B_Init.h"

#elif defined (HX8357C_DRIVER)
    #include "TFT_Drivers/HX8357C_Init.h"

#endif

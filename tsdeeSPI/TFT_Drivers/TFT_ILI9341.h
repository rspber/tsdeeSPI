
#pragma once

// Change the width and height if required (defined in portrait mode)
// or use the constructor to over-ride defaults

#if defined (ILI9341_DRIVER) || defined (ILI9341_2_DRIVER)
  #define TFT_WIDTH  240
  #define TFT_HEIGHT 320
#elif defined (ILI9342_DRIVER)
  #define TFT_WIDTH  320
  #define TFT_HEIGHT 240
#endif


// All ILI9341 specific commands some are used by init()
#define ILI9341_RDDID   0x04
#define ILI9341_RDDST   0x09

#define ILI9341_RDMODE  0x0A
#define ILI9341_RDMADCTL  0x0B
#define ILI9341_RDPIXFMT  0x0C
#define ILI9341_RDIMGFMT  0x0A
#define ILI9341_RDSELFDIAG  0x0F

#define ILI9341_GAMMASET    0x26    // Gamma set

#define ILI9341_WRDISBV  0x51
#define ILI9341_RDDISBV  0x52
#define ILI9341_WRCTRLD  0x53

#define ILI9341_FRMCTR1 0xB1
#define ILI9341_FRMCTR2 0xB2
#define ILI9341_FRMCTR3 0xB3
#define ILI9341_INVCTR  0xB4
#define ILI9341_DFUNCTR 0xB6

#define ILI9341_PWCTR1  0xC0
#define ILI9341_PWCTR2  0xC1
#define ILI9341_PWCTR3  0xC2
#define ILI9341_PWCTR4  0xC3
#define ILI9341_PWCTR5  0xC4
#define ILI9341_VMCTR1  0xC5
#define ILI9341_VMCTR2  0xC7

#define ILI9341_RDID4   0xD3
#define ILI9341_RDINDEX 0xD9
#define ILI9341_RDID1   0xDA
#define ILI9341_RDID2   0xDB
#define ILI9341_RDID3   0xDC
#define ILI9341_RDIDX   0xDD // TBC

#define ILI9341_GMCTRP1 0xE0
#define ILI9341_GMCTRN1 0xE1
#define ILI9341_PVGAMMA     0xE0    // Positive gamma correction
#define ILI9341_NVGAMMA     0xE1    // Negative gamma correction

#define ILI9341_EF          0xEF    // undocumented
#define ILI9341_INTFCCTR    0xF6    // Interface control
#define ILI9341_PUMPRC      0xF7    // Pump ratio control
#define ILI9341_PWCTR6      0xFC

// retired in version v.1.02
#define ILI9341_PWCTRA      0xCB    // Power control A
#define ILI9341_PWCTRB      0xCF    // Power control B
#define ILI9341_DTCA        0xE8    // Driver timing control A
// E9
#define ILI9341_DTCB        0xEA    // Driver timing control B
#define ILI9341_POSC        0xED    // Power on sequence control
#define ILI9341_ENABLE3G    0xF2    // Enable 3 gamma control

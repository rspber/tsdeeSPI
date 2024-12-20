
#pragma once

// Change the width and height if required (defined in portrait mode)
// or use the constructor to over-ride defaults
#define TFT_WIDTH  320
#define TFT_HEIGHT 480

#define HX8357_SETOSC    0xB0
#define HX8357_SETPWR1   0xB1
#define HX8357_SETRGB    0xB3
#define HX8357D_SETCYC   0xB4
#define HX8357D_SETCOM   0xB6
#define HX8357D_SETC     0xB9
#define HX8357D_SETSTBA  0xC0
#define HX8357_SETPANEL  0xCC
#define HX8357D_SETGAMMA 0xE0

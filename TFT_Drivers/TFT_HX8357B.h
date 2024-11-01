
#pragma once

// Change the width and height if required (defined in portrait mode)
// or use the constructor to over-ride defaults
#define TFT_WIDTH  320
#define TFT_HEIGHT 480

#define HX8357_PANELDRV   0xC0    // W/R (7) Panel Driving Setting
#define HX8357_FRAINCTL   0xC5    // W/R (1) Frame rate and Inversion Control
#define HX8357_GAMMASET   0xC8    // W/R (12) Gamma Setting
#define HX8357_POWERSET   0xD0    // W/R (3) Power Setting
#define HX8357_VCOMCTL    0xD1    // W/R (3) VCOM Control
#define HX8357_POWNORMD   0xD2    // W/R (2) Power Setting for Normal Mode

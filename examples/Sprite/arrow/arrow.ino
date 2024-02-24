/*
 TV example
*/

#include <TFT_eSPI.h>
#include "arrow.h"

TFT_eSPI tft;
TFT_eSprite ar = TFT_eSprite(&tft);
TFT_eSprite bg = TFT_eSprite(&tft);

int x = 20;

void setup() {
  tft.init();
  tft.setRotation(1);
  tft.setSwapBytes(true);

  ar.createSprite(96, 96);
  ar.setSwapBytes(true);

  bg.createSprite(320, 100);
}

void loop() {
  bg.fillSprite(TFT_PURPLE);
  ar.pushImage(0, 0, 96, 96, arrow_img);
  ar.pushToSprite(&bg, x, 5, TFT_BLACK);
  bg.pushSprite(0,0);
  
  if (++x > 330) {
    x = -100;
  }
}

/*
 TV example
*/

#include <TFT_eSPI.h>
#include "arrow.h"

TFT_eSPI tft;
TFT_eSprite bg(&tft);
TFT_eSprite ar(&tft);

int x = 20;

void setup() {
  Serial.begin(9600);

  tft.init();
  tft.setRotation(1);
//  tft.setSwapBytes(true);
  ar.setColorDepth(8);
  bg.setColorDepth(8);

  ar.createSprite(96, 96);
//  ar.setSwapBytes(true);

  bg.createSprite(320, 100);

  tft.fillScreen(RGB(255,0,0));
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

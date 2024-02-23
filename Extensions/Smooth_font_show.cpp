/*
  TFT_eSPI :: showFont adaptation to tsdeeSPI

  Copyright (c) 2024, rspber (https://github.com/rspber)

  Isolated from Smooth_font.cpp

*/

/***************************************************************************************
** Function name:           showFont
** Description:             Page through all characters in font, td ms between screens
*************************************************************************************x*/
void TFT_eSPI::showFont(uint32_t td)
{
  if(!fontLoaded) return;

  int16_t cursorX = width(); // Force start of new page to initialise cursor
  int16_t cursorY = height();// for the first character
  uint32_t timeDelay = 0;    // No delay before first page

  fillScreen(_textbgcolor);

  for (uint16_t i = 0; i < gFont.gCount; i++)
  {
    // Check if this will need a new screen
    if (cursorX + gdX[i] + gWidth[i] >= width())  {
      cursorX = -gdX[i];

      cursorY += gFont.yAdvance;
      if (cursorY + gFont.maxAscent + gFont.descent >= height()) {
        cursorX = -gdX[i];
        cursorY = 0;
        delay(timeDelay);
        timeDelay = td;
        fillScreen(_textbgcolor);
      }
    }

    setCursor(cursorX, cursorY);
    drawGlyph(_clip, _cursor, gUnicode[i], _textcolor, _textbgcolor);
    cursorX += gxAdvance[i];
    yield();
  }

  delay(timeDelay);
  fillScreen(_textbgcolor);
}



/***************************************************************************************
** Function name:           drawGlyph
** Description:             Write a character to the sprite cursor position
***************************************************************************************/
//
void TFT_eSprite::drawGlyph(wh_clip_t& clip, cursor_t& cursor, uint16_t code, rgb_t textcolor, rgb_t textbgcolor)
{
  uint16_t fg = textcolor;
  uint16_t bg = textbgcolor;
  bool getBG  = false;
  if (fg == bg) getBG = true;

  // Check if cursor has moved
  if (_last_cursor_x != cursor.x)
  {
    _bg_cursor_x = cursor.x;
    _last_cursor_x = cursor.x;
  }

  if (code < 0x21)
  {
    if (code == 0x20) {
      if (_fillbg) fillRect(clip, _bg_cursor_x, cursor.y, (cursor.x + gFont.spaceWidth) - _bg_cursor_x, gFont.yAdvance, bg);
      cursor.x += gFont.spaceWidth;
      _bg_cursor_x = cursor.x;
      _last_cursor_x = cursor.x;
      return;
    }

    if (code == '\n') {
      cursor.x = 0;
      _bg_cursor_x = 0;
      _last_cursor_x = 0;
      cursor.y += gFont.yAdvance;
      if (_textwrapY && (cursor.y >= height())) cursor.y = 0;
      return;
    }
  }

  uint16_t gNum = 0;
  bool found = getUnicodeIndex(code, &gNum);

  if (found)
  {

    bool newSprite = !_created;

    if (newSprite)
    {
      createSprite(gWidth[gNum], gFont.yAdvance);
      if(fg != bg) fillSprite(bg);
      cursor.x = -gdX[gNum];
      _bg_cursor_x = cursor.x;
      _last_cursor_x = cursor.x;
      cursor.y = 0;
    }
    else
    {
      if (_textwrapX && ((cursor.x + gWidth[gNum] + gdX[gNum]) > width())) {
        cursor.y += gFont.yAdvance;
        cursor.x = 0;
        _bg_cursor_x = 0;
        _last_cursor_x = 0;
      }

      if (_textwrapY && ((cursor.y + gFont.yAdvance) > height())) cursor.y = 0;
      if (cursor.x == 0) cursor.x -= gdX[gNum];
    }

    uint8_t* pbuffer = nullptr;
    const uint8_t* gPtr = (const uint8_t*) gFont.gArray;

#ifdef FONT_FS_AVAILABLE
    if (fs_font) {
      fontFile.seek(gBitmap[gNum], fs::SeekSet); // This is slow for a significant position shift!
      pbuffer =  (uint8_t*)malloc(gWidth[gNum]);
    }
#endif

    int16_t cy = cursor.y + gFont.maxAscent - gdY[gNum];
    int16_t cx = cursor.x + gdX[gNum];

    //  if (cx > width() && bg_cursor_x > width()) return;
    //  if (cursor_y > height()) return;

    int16_t  fxs = cx;
    uint32_t fl = 0;
    int16_t  bxs = cx;
    uint32_t bl = 0;
    int16_t  bx = 0;
    uint8_t pixel = 0;

    int16_t fillwidth  = 0;
    int16_t fillheight = 0;

    // Fill area above glyph
    if (_fillbg) {
      fillwidth  = (cursor.x + gxAdvance[gNum]) - _bg_cursor_x;
      if (fillwidth > 0) {
        fillheight = gFont.maxAscent - gdY[gNum];
        if (fillheight > 0) {
          fillRect(clip, _bg_cursor_x, cursor.y, fillwidth, fillheight, textbgcolor);
        }
      }
      else {
        // Could be negative
        fillwidth = 0;
      }

      // Fill any area to left of glyph
      if (_bg_cursor_x < cx) fillRect(clip, _bg_cursor_x, cy, cx - _bg_cursor_x, gHeight[gNum], textbgcolor);
      // Set x position in glyph area where background starts
      if (_bg_cursor_x > cx) bx = _bg_cursor_x - cx;
      // Fill any area to right of glyph
      if (cx + gWidth[gNum] < cursor.x + gxAdvance[gNum]) {
        fillRect(clip, cx + gWidth[gNum], cy, (cursor.x + gxAdvance[gNum]) - (cx + gWidth[gNum]), gHeight[gNum], textbgcolor);
      }
    }

    for (int32_t y = 0; y < gHeight[gNum]; y++)
    {
#ifdef FONT_FS_AVAILABLE
      if (fs_font) {
        fontFile.read(pbuffer, gWidth[gNum]);
      }
#endif

      for (int32_t x = 0; x < gWidth[gNum]; x++)
      {
#ifdef FONT_FS_AVAILABLE
        if (fs_font) pixel = pbuffer[x];
        else
#endif
        pixel = pgm_read_byte(gPtr + gBitmap[gNum] + x + gWidth[gNum] * y);

        if (pixel)
        {
          if (bl) { drawFastHLine(clip, bxs, y + cy, bl, bg); bl = 0; }
          if (pixel != 0xFF)
          {
            if (fl) {
              if (fl==1) drawPixel(clip, fxs, y + cy, fg);
              else drawFastHLine(clip, fxs, y + cy, fl, fg);
              fl = 0;
            }
            if (getBG) bg = readPixel(clip, x + cx, y + cy);
            drawPixel(clip, x + cx, y + cy, alphaBlend(pixel, fg, bg));
          }
          else
          {
            if (fl==0) fxs = x + cx;
            fl++;
          }
        }
        else
        {
          if (fl) { drawFastHLine(clip, fxs, y + cy, fl, fg); fl = 0; }
          if (_fillbg) {
            if (x >= bx) {
              if (bl==0) bxs = x + cx;
              bl++;
            }
          }
        }
      }
      if (fl) { drawFastHLine(clip, fxs, y + cy, fl, fg); fl = 0; }
      if (bl) { drawFastHLine(clip, bxs, y + cy, bl, bg); bl = 0; }
    }

    // Fill area below glyph
    if (fillwidth > 0) {
      fillheight = (cursor.y + gFont.yAdvance) - (cy + gHeight[gNum]);
      if (fillheight > 0) {
        fillRect(clip, _bg_cursor_x, cy + gHeight[gNum], fillwidth, fillheight, textbgcolor);
      }
    }

    if (pbuffer) free(pbuffer);
    cursor.x += gxAdvance[gNum];

    if (newSprite)
    {
      pushSprite(cx, cursor.y);
      deleteSprite();
    }
  }
  else
  {
    // Not a Unicode in font so draw a rectangle and move on cursor
    drawRect(clip, cursor.x, cursor.y + gFont.maxAscent - gFont.ascent, gFont.spaceWidth, gFont.ascent, fg);
    cursor.x += gFont.spaceWidth + 1;
  }
  _bg_cursor_x = cursor.x;
  _last_cursor_x = cursor.x;
}


/***************************************************************************************
** Function name:           printToSprite
** Description:             Write a string to the sprite cursor position
***************************************************************************************/
void TFT_eSprite::printToSprite(String string)
{
  if(!fontLoaded) return;
  printToSprite((char*)string.c_str(), string.length());
}


/***************************************************************************************
** Function name:           printToSprite
** Description:             Write a string to the sprite cursor position
***************************************************************************************/
void TFT_eSprite::printToSprite(char *cbuffer, uint16_t len) //String string)
{
  if(!fontLoaded) return;

  uint16_t n = 0;
  bool newSprite = !_created;
  int16_t  cursorX = _tft->getCursorX();

  if (newSprite)
  {
    int16_t sWidth = 0;
    uint16_t index = 0;
    bool     first = true;
    while (n < len)
    {
      uint16_t unicode = decodeUTF8((uint8_t*)cbuffer, &n, len - n);
      if (getUnicodeIndex(unicode, &index))
      {
        if (first) {
          first = false;
          sWidth -= gdX[index];
          cursorX += gdX[index];
        }
        if (n == len) sWidth += ( gWidth[index] + gdX[index]);
        else sWidth += gxAdvance[index];
      }
      else sWidth += gFont.spaceWidth + 1;
    }

    createSprite(sWidth, gFont.yAdvance);

    if (_textcolor != _textbgcolor) fillSprite(_textbgcolor);
  }

  n = 0;

  while (n < len)
  {
    uint16_t unicode = decodeUTF8((uint8_t*)cbuffer, &n, len - n);
    //Serial.print("Decoded Unicode = 0x");Serial.println(unicode,HEX);
    //Serial.print("n = ");Serial.println(n);
    drawGlyph(_clip, _cursor, unicode, _textcolor, _textbgcolor);
  }

  if (newSprite)
  { // The sprite had to be created so place at TFT cursor
    pushSprite(cursorX, _tft->getCursorY());
    deleteSprite();
  }
}


/***************************************************************************************
** Function name:           printToSprite
** Description:             Print character in a Sprite, create sprite if needed
***************************************************************************************/
int16_t TFT_eSprite::printToSprite(int16_t x, int16_t y, uint16_t index)
{
  bool newSprite = !_created;
  int16_t sWidth = gWidth[index];

  if (newSprite)
  {
    createSprite(sWidth, gFont.yAdvance);

    if (_textcolor != _textbgcolor) fillSprite(_textbgcolor);

    drawGlyph(_clip, _cursor, gUnicode[index], _textcolor, _textbgcolor);

    pushSprite(x + gdX[index], y, _textbgcolor);
    deleteSprite();
  }

  else drawGlyph(_clip, _cursor, gUnicode[index], _textcolor, _textbgcolor);

  return gxAdvance[index];
}

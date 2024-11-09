
/***************************************************************************************
** Function name:           drawChar
** Description:             draw a single character in the Adafruit GLCD or freefont
***************************************************************************************/
void TFT_eSprite::drawChar_GLCD_GFXFF(clip_t& clip, cursor_t& cursor, chr_font_t& font, uint16_t c, rgb_t color, rgb_t bg)
{
  Serial.println("drawChar_GLCD_GFXFF");

  if ( !_created ) return;

  if (c < 32) return;
#ifdef LOAD_GLCD
//>>>>>>>>>>>>>>>>>>
#ifdef LOAD_GFXFF
  if (!font.gfxFont) { // 'Classic' built-in font
#endif
//>>>>>>>>>>>>>>>>>>

  int32_t xd = cursor.x + clip.xDatum;
  int32_t yd = cursor.y + clip.yDatum;

  if ((xd >= clip.x2) || // Clip right
      (yd >= clip.y2))   // Clip bottom
    return;

  if ((xd + 6 * font.size - 1 < clip.x1) || // Clip left
      (yd + 8 * font.size - 1 < clip.y1))   // Clip top
    return;

  if (c > 255) return;
  if (!_cp437 && c > 175) c++;

  bool fillbg = (bg != color);

  if (font.size == 1 && fillbg)
  {
    uint8_t column[6];
    uint8_t mask = 0x1;

    for (int8_t i = 0; i < 5; i++ ) column[i] = pgm_read_byte(glcdfont + (c * 5) + i);
    column[5] = 0;

    int8_t j, k;
    for (j = 0; j < 8; j++) {
      for (k = 0; k < 5; k++ ) {
        if (column[k] & mask) {
          drawPixel(clip, cursor.x + k, cursor.y + j, color);
        }
        else {
          drawPixel(clip, cursor.x + k, cursor.y + j, bg);
        }
      }

      mask <<= 1;

      drawPixel(clip, cursor.x + k, cursor.y + j, bg);
    }
  }
  else
  {
    for (int8_t i = 0; i < 6; i++ ) {
      uint8_t line;
      if (i == 5)
        line = 0x0;
      else
        line = pgm_read_byte(glcdfont + (c * 5) + i);

      if (font.size == 1) // default size
      {
        for (int8_t j = 0; j < 8; j++) {
          if (line & 0x1) drawPixel(clip, cursor.x + i, cursor.y + j, color);
          line >>= 1;
        }
      }
      else {  // big size
        for (int8_t j = 0; j < 8; j++) {
          if (line & 0x1) fillRect(clip, cursor.x + (i * font.size), cursor.y + (j * font.size), font.size, font.size, color);
          else if (fillbg) fillRect(clip, cursor.x + i * font.size, cursor.y + j * font.size, font.size, font.size, bg);
          line >>= 1;
        }
      }
    }
  }

//>>>>>>>>>>>>>>>>>>>>>>>>>>>
#ifdef LOAD_GFXFF
  } else { // Custom font
#endif
//>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif // LOAD_GLCD

#ifdef LOAD_GFXFF
    // Filter out bad characters not present in font
    if ((c >= pgm_read_word(&font.gfxFont->first)) && (c <= pgm_read_word(&font.gfxFont->last )))
    {
//>>>>>>>>>>>>>>>>>>>>>>>>>>>

      c -= pgm_read_word(&font.gfxFont->first);
      GFXglyph *glyph  = &(((GFXglyph *)pgm_read_dword(&font.gfxFont->glyph))[c]);

      uint8_t  w  = pgm_read_byte(&glyph->width),
               h  = pgm_read_byte(&glyph->height);
      int8_t   xo = pgm_read_byte(&glyph->xOffset),
               yo = pgm_read_byte(&glyph->yOffset);

      if ((cursor.x + xo + w * font.size - 1 + clip.xDatum < clip.x1) || // Clip left
          (cursor.y + yo + h * font.size - 1 + clip.yDatum < clip.y1))   // Clip top
        return;

      uint8_t  *bitmap = (uint8_t *)pgm_read_dword(&font.gfxFont->bitmap);
      uint32_t bo = pgm_read_word(&glyph->bitmapOffset);

      uint8_t  xx, yy, bits=0, bit=0;
      //uint8_t  xa = pgm_read_byte(&glyph->xAdvance);
      int16_t  xo16 = 0, yo16 = 0;

      if(font.size > 1) {
        xo16 = xo;
        yo16 = yo;
      }

      uint16_t hpc = 0; // Horizontal foreground pixel count
      for(yy=0; yy<h; yy++) {
        for(xx=0; xx<w; xx++) {
          if(bit == 0) {
            bits = pgm_read_byte(&bitmap[bo++]);
            bit  = 0x80;
          }
          if(bits & bit) hpc++;
          else {
            if (hpc) {
              if(font.size == 1) drawFastHLine(clip, cursor.x+xo+xx-hpc, cursor.y+yo+yy, hpc, color);
              else fillRect(clip, cursor.x+(xo16+xx-hpc)*font.size, cursor.y+(yo16+yy)*font.size, font.size*hpc, font.size, color);
              hpc=0;
            }
          }
          bit >>= 1;
        }
        // Draw pixels for this line as we are about to increment yy
        if (hpc) {
          if(font.size == 1) drawFastHLine(clip, cursor.x+xo+xx-hpc, cursor.y+yo+yy, hpc, color);
          else fillRect(clip, cursor.x+(xo16+xx-hpc)*font.size, cursor.y+(yo16+yy)*font.size, font.size*hpc, font.size, color);
          hpc=0;
        }
      }
    }
#endif
  }
}

void TFT_eSprite::drawCharRLEfont(int32_t xd, int32_t yd, int32_t pY, uint16_t width, uint16_t height, int16_t textsize, rgb_t textcolor, uint32_t flash_address)
{
  Serial.println("drawCharRLEfont");

    int32_t w = width;
    w *= height; // Now w is total number of pixels in the character

      int32_t px = 0, py = pY; // To hold character block start and end column and row values
      int32_t pc = 0; // Pixel count
      int16_t np = textsize * textsize; // Number of pixels in a drawn pixel

      // 16-bit pixel count so maximum font size is equivalent to 180x180 pixels in area
      // w is total number of pixels to plot to fill character block
      while (pc < w) {
        uint8_t line = pgm_read_byte((uint8_t *)flash_address);
        flash_address++;
        if (line & 0x80) {
          line &= 0x7F;
          line++;
          if (textsize > 1) {
            px = xd + textsize * (pc % width); // Keep these px and py calculations outside the loop as they are slow
            py = yd + textsize * (pc / width);
          }
          else {
            px = xd + pc % width; // Keep these px and py calculations outside the loop as they are slow
            py = yd + pc / width;
          }
          while (line--) { // In this case the while(line--) is faster
            pc++; // This is faster than putting pc+=line before while()?
            setWindow(px, py, textsize, textsize);

            if (textsize > 1) {
              int16_t j = np;
              while (j--) writeColor(textcolor);
            }
            else writeColor(textcolor);

            px += textsize;

            if (px >= xd + width * textsize) {
              px = xd;
              py += textsize;
            }
          }
        }
        else {
          line++;
          pc += line;
        }
      }
}


void TFT_eSprite::drawCharRLE_1(int32_t width, int32_t height, rgb_t textcolor, rgb_t textbgcolor, uint32_t flash_address)
{
  Serial.println("drawCharRLE_1");

        int32_t w = width;
        w *= height; // Now w is total number of pixels in the character

        // Maximum font size is equivalent to 180x180 pixels in area
        while (w > 0) {
          uint8_t line = pgm_read_byte((uint8_t *)flash_address++); // 8 bytes smaller when incrementing here
          if (line & 0x80) {
            line &= 0x7F;
            line++; w -= line;
            while (line--) writeColor(textcolor);
          }
          else {
            line++; w -= line;
            while (line--) writeColor(textbgcolor);
          }
        }
}

/***************************************************************************************
** Function name:           drawChar
** Description:             draw a unicode glyph into the sprite
***************************************************************************************/
  // TODO: Rationalise with TFT_eSPI
  // Any UTF-8 decoding must be done before calling drawChar()
int16_t TFT_eSprite::drawChar(clip_t& clip, cursor_t& cursor, chr_font_t& font, uint16_t uniCode, rgb_t textcolor, rgb_t textbgcolor)
{
  Serial.println("drawChar");

  if (!uniCode) return 0;

  if (font.font == 1) {
#ifdef LOAD_GLCD
  #ifndef LOAD_GFXFF
    drawChar_GLCD_GFXFF(clip, cursor, font, uniCode, textcolor, textbgcolor);
    return 6 * font.size;
  #endif
#else
  #ifndef LOAD_GFXFF
    return 0;
  #endif
#endif

#ifdef LOAD_GFXFF
    drawChar_GLCD_GFXFF(clip, cursor, font, uniCode, textcolor, textbgcolor);
    if (!font.gfxFont) { // 'Classic' built-in font
    #ifdef LOAD_GLCD
      return 6 * font.size;
    #else
      return 0;
    #endif
    }
    else {
      if((uniCode >= pgm_read_word(&font.gfxFont->first)) && (uniCode <= pgm_read_word(&font.gfxFont->last) )) {
        uint16_t   c2    = uniCode - pgm_read_word(&font.gfxFont->first);
        GFXglyph *glyph = &(((GFXglyph *)pgm_read_dword(&font.gfxFont->glyph))[c2]);
        return pgm_read_byte(&glyph->xAdvance) * font.size;
      }
      else {
        return 0;
      }
    }
#endif
  }

  if ((font.font > 1) && (font.font < 9) && ((uniCode < 32) || (uniCode > 127))) return 0;

  int32_t width  = 0;
  int32_t height = 0;
  uint32_t flash_address = 0;
  uniCode -= 32;

#ifdef LOAD_FONT2
  if (font.font == 2) {
    flash_address = pgm_read_dword(&chrtbl_f16[uniCode]);
    width = pgm_read_byte(widtbl_f16 + uniCode);
    height = chr_hgt_f16;
  }
  #ifdef LOAD_RLE
  else
  #endif
#endif

#ifdef LOAD_RLE
  {
    if ((font.font>2) && (font.font<9)) {
      flash_address = pgm_read_dword( (const void*)(pgm_read_dword( &(fontdata[font.font].chartbl ) ) + uniCode*sizeof(void *)) );
      width = pgm_read_byte( (uint8_t *)pgm_read_dword( &(fontdata[font.font].widthtbl ) ) + uniCode );
      height= pgm_read_byte( &fontdata[font.font].height );
    }
  }
#endif

  int32_t xd = cursor.x + clip.xDatum;
  int32_t yd = cursor.y + clip.yDatum;

  if ((xd + width * font.size < clip.x1 || xd >= clip.x2) && (yd + height * font.size < clip.y1 || yd >= clip.y2)) return width * font.size ;

  bool offclip = xd < clip.x1 || xd + width  * font.size >= clip.x2 || yd < clip.y1 || yd + height * font.size >= clip.y2;

#ifdef LOAD_FONT2 // chop out code if we do not need it
  if (font.font == 2) {
    drawCharFont2(clip, cursor, width, height, font.size, textcolor, textbgcolor, flash_address);
  }

  #ifdef LOAD_RLE
  else
  #endif
#endif  //FONT2

#ifdef LOAD_RLE  //674 bytes of code
  // Font is not 2 and hence is RLE encoded
  {
    if (textcolor == textbgcolor && !offclip && _bpp != 1) {
      drawCharRLEfont(xd, yd, cursor.y, width, height, font.size, textcolor, flash_address);
    }
    else {
      // Text colour != background and font.size = 1 and character is within viewport area
      // so use faster drawing of characters and background using block write
      if (textcolor != textbgcolor && font.size == 1 && !offclip && _bpp != 1) {
        setWindow(xd, yd, width, height);
        drawCharRLE_1(width, height, textcolor, textbgcolor, flash_address);
      }
      else {
        drawCharRLE_3(clip, cursor, width, height, font.size, textcolor, textbgcolor, flash_address);
      }
    }
  }
  // End of RLE font rendering
#endif

  return width * font.size;    // x +
}

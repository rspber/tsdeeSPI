/*
  Character drawing level

  Copyright (c) 2024, rspber (https://github.com/rspber)

  Based on: TFT_eSPI

  Originally notes below:
*/

/***************************************************
  Arduino TFT graphics library targeted at 32-bit
  processors such as ESP32, ESP8266 and STM32.

  This is a stand-alone library that contains the
  hardware driver, the graphics functions and the
  proportional fonts.

  The larger fonts are Run Length Encoded to reduce their
  size.

  Created by Bodmer 2/12/16
  Last update by Bodmer 20/03/20
 ****************************************************/

#include "TFT_CHAR.h"

/***************************************************************************************
** Function name:           TFT_eSPI
** Description:             Constructor , we must use hardware SPI pins
***************************************************************************************/
TFT_CHAR::TFT_CHAR() : TFT_GFX()
{
  _fillbg    = false;   // Smooth font only at the moment, force text background fill

  _isDigits   = false;   // No bounding box adjustment
  _textwrapX  = true;    // Wrap text at end of line when using print stream
  _textwrapY  = false;   // Wrap text at bottom of screen when using print stream

  _fontsloaded = 0;

  _cp437    = false;    // Legacy GLCD font bug fix disabled by default
  _utf8     = true;     // UTF8 decoding enabled

#if defined (FONT_FS_AVAILABLE) && defined (SMOOTH_FONT)
  fs_font  = true;     // Smooth font filing system or array (fs_font = false) flag
#endif

// Flags for which fonts are loaded
#ifdef LOAD_GLCD
  _fontsloaded  = 0x0002; // Bit 1 set
#endif

#ifdef LOAD_FONT2
  _fontsloaded |= 0x0004; // Bit 2 set
#endif

#ifdef LOAD_FONT4
  _fontsloaded |= 0x0010; // Bit 4 set
#endif

#ifdef LOAD_FONT6
  _fontsloaded |= 0x0040; // Bit 6 set
#endif

#ifdef LOAD_FONT7
  _fontsloaded |= 0x0080; // Bit 7 set
#endif

#ifdef LOAD_FONT8
  _fontsloaded |= 0x0100; // Bit 8 set
#endif

#ifdef LOAD_FONT8N
  _fontsloaded |= 0x0200; // Bit 9 set
#endif

#ifdef SMOOTH_FONT
  _fontsloaded |= 0x8000; // Bit 15 set
#endif
}


void TFT_CHAR::setCallback(getColorCallback getCol)
{
  getColor = getCol;
}


/***************************************************************************************
** Function name:           textWidth
** Description:             Return the width in pixels of a string in a given font
***************************************************************************************/
int16_t TFT_CHAR::textWidth(chr_font_t& font, const char *string)
{
  int32_t str_width = 0;
  uint16_t uniCode  = 0;

#ifdef SMOOTH_FONT
  if(fontLoaded) {
    while (*string) {
      uniCode = decodeUTF8(*string++);
      if (uniCode) {
        if (uniCode == 0x20) str_width += gFont.spaceWidth;
        else {
          uint16_t gNum = 0;
          bool found = getUnicodeIndex(uniCode, &gNum);
          if (found) {
            if(str_width == 0 && gdX[gNum] < 0) str_width -= gdX[gNum];
            if (*string || _isDigits) str_width += gxAdvance[gNum];
            else str_width += (gdX[gNum] + gWidth[gNum]);
          }
          else str_width += gFont.spaceWidth + 1;
        }
      }
    }
    _isDigits = false;
    return str_width;
  }
#endif

  if (font.font > 1 && font.font < 9) {
    char *widthtable = (char *)pgm_read_dword( &(fontdata[font.font].widthtbl ) ) - 32; //subtract the 32 outside the loop

    while (*string) {
      uniCode = *(string++);
      if (uniCode > 31 && uniCode < 128)
      str_width += pgm_read_byte( widthtable + uniCode); // Normally we need to subtract 32 from uniCode
      else str_width += pgm_read_byte( widthtable + 32); // Set illegal character = space width
    }

  }
  else {

#ifdef LOAD_GFXFF
    if(font.gfxFont) { // New font
      while (*string) {
        uniCode = decodeUTF8(*string++);
        if ((uniCode >= pgm_read_word(&font.gfxFont->first)) && (uniCode <= pgm_read_word(&font.gfxFont->last ))) {
          uniCode -= pgm_read_word(&font.gfxFont->first);
          GFXglyph *glyph  = &(((GFXglyph *)pgm_read_dword(&font.gfxFont->glyph))[uniCode]);
          // If this is not the  last character or is a digit then use xAdvance
          if (*string  || _isDigits) str_width += pgm_read_byte(&glyph->xAdvance);
          // Else use the offset plus width since this can be bigger than xAdvance
          else str_width += ((int8_t)pgm_read_byte(&glyph->xOffset) + pgm_read_byte(&glyph->width));
        }
      }
    }
    else
#endif
    {
#ifdef LOAD_GLCD
      while (*string++) str_width += 6;
#endif
    }
  }
  _isDigits = false;
  return str_width * font.size;
}


/***************************************************************************************
** Function name:           fontsLoaded
** Description:             return an encoded 16-bit value showing the fonts loaded
***************************************************************************************/
// Returns a value showing which fonts are loaded (bit N set =  Font N loaded)
uint16_t TFT_CHAR::fontsLoaded(void)
{
  return _fontsloaded;
}


/***************************************************************************************
** Function name:           fontHeight
** Description:             return the height of a font (yAdvance for free fonts)
***************************************************************************************/
int16_t TFT_CHAR::fontHeight(chr_font_t& font)
{
  if (font.font > 8) return 0;

#ifdef SMOOTH_FONT
  if(fontLoaded) return gFont.yAdvance;
#endif

#ifdef LOAD_GFXFF
  if (font.font == 1) {
    if(font.gfxFont) { // New font
      return pgm_read_byte(&font.gfxFont->yAdvance) * font.size;
    }
  }
#endif
  return pgm_read_byte( &fontdata[font.font].height ) * font.size;
}


/***************************************************************************************
** Function name:           drawChar
** Description:             draw a single character in the GLCD or GFXFF font
***************************************************************************************/
void TFT_CHAR::drawChar_GLCD_GFXFF(clip_t& clip, cursor_t& cursor, chr_font_t& font, uint16_t c, rgb_t color, rgb_t bg)
{
#ifdef LOAD_GLCD
//>>>>>>>>>>>>>>>>>>
  #ifdef LOAD_GFXFF
  if (!font.gfxFont) { // 'Classic' built-in GLCD font
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
  bool offclip = xd < clip.x1 || xd + 6  * font.size >= clip.x2 || yd < clip.y1 || yd + 8 * font.size >= clip.y2;

  if (font.size == 1 && fillbg && !offclip) {
    uint8_t column[6];
    uint8_t mask = 0x1;
    begin_tft_write();

    setWindow(xd, yd, 6, 8);

    drawCharDefault(&glcdfont[0] + (c * 5), color, bg);

    end_tft_write();
  }
  else {
    //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
    inTransaction = true;

    for (int8_t i = 0; i < 6; i++ ) {
      uint8_t line;
      if (i == 5)
        line = 0x0;
      else
        line = pgm_read_byte(&glcdfont[0] + (c * 5) + i);

      if (font.size == 1 && !fillbg) { // default size
        for (int8_t j = 0; j < 8; j++) {
          if (line & 0x1) drawPixel(clip, cursor.x + i, cursor.y + j, color);
          line >>= 1;
        }
      }
      else {  // big size or clipped
        for (int8_t j = 0; j < 8; j++) {
          if (line & 0x1) fillRect(clip, cursor.x + (i * font.size), cursor.y + (j * font.size), font.size, font.size, color);
          else if (fillbg) fillRect(clip, cursor.x + i * font.size, cursor.y + j * font.size, font.size, font.size, bg);
          line >>= 1;
        }
      }
    }
    inTransaction = lockTransaction;
    end_tft_write();              // Does nothing if Sprite class uses this function
  }

//>>>>>>>>>>>>>>>>>>>>>>>>>>>
  #ifdef LOAD_GFXFF
  } else { // Custom font
  #endif
//>>>>>>>>>>>>>>>>>>>>>>>>>>>
#endif // LOAD_GLCD

#ifdef LOAD_GFXFF
    // Filter out bad characters not present in font
    if ((c >= pgm_read_word(&font.gfxFont->first)) && (c <= pgm_read_word(&font.gfxFont->last ))) {
      //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
      inTransaction = true;
//>>>>>>>>>>>>>>>>>>>>>>>>>>>

      c -= pgm_read_word(&font.gfxFont->first);
      GFXglyph *glyph  = &(((GFXglyph *)pgm_read_dword(&font.gfxFont->glyph))[c]);
      uint8_t  *bitmap = (uint8_t *)pgm_read_dword(&font.gfxFont->bitmap);

      uint32_t bo = pgm_read_word(&glyph->bitmapOffset);
      uint8_t  w  = pgm_read_byte(&glyph->width),
               h  = pgm_read_byte(&glyph->height);
               //xa = pgm_read_byte(&glyph->xAdvance);
      int8_t   xo = pgm_read_byte(&glyph->xOffset),
               yo = pgm_read_byte(&glyph->yOffset);
      uint8_t  xx, yy, bits=0, bit=0;
      int16_t  xo16 = 0, yo16 = 0;

      if (font.size > 1) {
        xo16 = xo;
        yo16 = yo;
      }

      // GFXFF rendering speed up
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

      inTransaction = lockTransaction;
      end_tft_write();              // Does nothing if Sprite class uses this function
    }
#endif
  }
}


/***************************************************************************************
** Function name:           decodeUTF8
** Description:             Serial UTF-8 decoder with fall-back to extended ASCII
*************************************************************************************x*/
uint16_t TFT_CHAR::decodeUTF8(uint8_t c)
{
  if (!_utf8) return c;

  // 7-bit Unicode Code Point
  if ((c & 0x80) == 0x00) {
    decoderState = 0;
    return c;
  }

  if (decoderState == 0) {
    // 11-bit Unicode Code Point
    if ((c & 0xE0) == 0xC0) {
      decoderBuffer = ((c & 0x1F)<<6);
      decoderState = 1;
      return 0;
    }
    // 16-bit Unicode Code Point
    if ((c & 0xF0) == 0xE0) {
      decoderBuffer = ((c & 0x0F)<<12);
      decoderState = 2;
      return 0;
    }
    // 21-bit Unicode Code Point not supported so fall-back to extended ASCII
    // if ((c & 0xF8) == 0xF0) return c;
  }
  else {
    if (decoderState == 2) {
      decoderBuffer |= ((c & 0x3F)<<6);
      decoderState--;
      return 0;
    }
    else {
      decoderBuffer |= (c & 0x3F);
      decoderState = 0;
      return decoderBuffer;
    }
  }

  decoderState = 0;

  return c; // fall-back to extended ASCII
}


/***************************************************************************************
** Function name:           decodeUTF8
** Description:             Line buffer UTF-8 decoder with fall-back to extended ASCII
*************************************************************************************x*/
uint16_t TFT_CHAR::decodeUTF8(uint8_t *buf, uint16_t *index, uint16_t remaining)
{
  uint16_t c = buf[(*index)++];
  //Serial.print("Byte from string = 0x"); Serial.println(c, HEX);

  if (!_utf8) return c;

  // 7-bit Unicode
  if ((c & 0x80) == 0x00) return c;

  // 11-bit Unicode
  if (((c & 0xE0) == 0xC0) && (remaining > 1))
    return ((c & 0x1F)<<6) | (buf[(*index)++]&0x3F);

  // 16-bit Unicode
  if (((c & 0xF0) == 0xE0) && (remaining > 2)) {
    c = ((c & 0x0F)<<12) | ((buf[(*index)++]&0x3F)<<6);
    return  c | ((buf[(*index)++]&0x3F));
  }

  // 21-bit Unicode not supported so fall-back to extended ASCII
  // if (((c & 0xF8) == 0xF0) && (remaining > 3)) {
  // c = ((c & 0x07) << 18) | ((buf[(*index)++] & 0x03F) << 12);
  // c |= ((buf[(*index)++] & 0x3F) << 6);
  // return c | ((buf[(*index)++] & 0x3F));

  return c; // fall-back to extended ASCII
}


/***************************************************************************************
** Function name:           write
** Description:             draw characters piped through serial stream
***************************************************************************************/
size_t TFT_CHAR::write(wh_clip_t& clip, cursor_t& cursor, chr_font_t& font, uint8_t utf8, rgb_t textcolor, rgb_t textbgcolor)
{
  uint16_t uniCode = decodeUTF8(utf8);

  if (!uniCode) return 1;

  if (utf8 == '\r') return 1;

#ifdef SMOOTH_FONT
  if(fontLoaded) {
    if (uniCode < 32 && utf8 != '\n') return 1;

    drawGlyph(clip, cursor, uniCode, textcolor, textbgcolor);

    return 1;
  }
#endif

  if (uniCode == '\n') uniCode+=22; // Make it a valid space character to stop errors

  uint16_t cwidth = 0;
  uint16_t cheight = 0;

//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv DEBUG vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
  //Serial.print((uint8_t) uniCode); // Debug line sends all printed TFT text to serial port
  //Serial.println(uniCode, HEX); // Debug line sends all printed TFT text to serial port
  //delay(5);                     // Debug optional wait for serial port to flush through
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ DEBUG ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#ifdef LOAD_GFXFF
  if (!font.gfxFont) {
#endif
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#ifdef LOAD_FONT2
  if (font.font == 2) {
    if (uniCode < 32 || uniCode > 127) return 1;

    cwidth = pgm_read_byte(widtbl_f16 + uniCode-32);
    cheight = chr_hgt_f16;
    // Font 2 is rendered in whole byte widths so we must allow for this
    cwidth = (cwidth + 6) / 8;  // Width in whole bytes for font 2, should be + 7 but must allow for font width change
    cwidth = cwidth * 8;        // Width converted back to pixels
  }
  #ifdef LOAD_RLE
  else
  #endif
#endif

#ifdef LOAD_RLE
  {
    if ((font.font > 2) && (font.font < 9)) {
      if (uniCode < 32 || uniCode > 127) return 1;
      // Uses the fontinfo struct array to avoid lots of 'if' or 'switch' statements
      cwidth = pgm_read_byte( (uint8_t *)pgm_read_dword( &(fontdata[font.font].widthtbl ) ) + uniCode-32 );
      cheight= pgm_read_byte( &fontdata[font.font].height );
    }
  }
#endif

#ifdef LOAD_GLCD
  if (font.font == 1) {
      cwidth =  6;
      cheight = 8;
  }
#else
  if (font.font == 1) return 1;
#endif

  cheight = cheight * font.size;

  if (utf8 == '\n') {
    cursor.y += cheight;
    cursor.x  = 0;
  }
  else {
    if (_textwrapX && (cursor.x + cwidth * font.size > clip.width)) {
      cursor.y += cheight;
      cursor.x = 0;
    }
    if (_textwrapY && (cursor.y >= (int32_t) clip.height)) cursor.y = 0;
    cursor.x += drawChar(clip, cursor, font, uniCode, textcolor, textbgcolor);
  }

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#ifdef LOAD_GFXFF
  } // Custom GFX font
  else {
    if(utf8 == '\n') {
      cursor.x  = 0;
      cursor.y += (int16_t)font.size * (uint8_t)pgm_read_byte(&font.gfxFont->yAdvance);
    } else {
      if (uniCode > pgm_read_word(&font.gfxFont->last )) return 1;
      if (uniCode < pgm_read_word(&font.gfxFont->first)) return 1;

      uint16_t   c2    = uniCode - pgm_read_word(&font.gfxFont->first);
      GFXglyph *glyph = &(((GFXglyph *)pgm_read_dword(&font.gfxFont->glyph))[c2]);
      uint8_t   w     = pgm_read_byte(&glyph->width),
                h     = pgm_read_byte(&glyph->height);
      if((w > 0) && (h > 0)) { // Is there an associated bitmap?
        int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset);
        if(_textwrapX && ((cursor.x + font.size * (xo + w)) > clip.width)) {
          // Drawing character would go off right edge; wrap to new line
          cursor.x  = 0;
          cursor.y += (int16_t)font.size * (uint8_t)pgm_read_byte(&font.gfxFont->yAdvance);
        }
        if (_textwrapY && (cursor.y >= (int32_t) clip.height)) cursor.y = 0;
        drawChar(clip, cursor, font, uniCode, textcolor, textbgcolor);
      }
      cursor.x += pgm_read_byte(&glyph->xAdvance) * (int16_t)font.size;
    }
  }
#endif // LOAD_GFXFF
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

  return 1;
}


void TFT_CHAR::drawCharFont2(clip_t& clip, cursor_t& cursor, int32_t width, int32_t height, uint8_t textsize, rgb_t textcolor, rgb_t textbgcolor, uint32_t flash_address)
{
      int32_t pY = cursor.y;
      int32_t w = width;
      w = w + 6; // Should be + 7 but we need to compensate for width increment
      w = w / 8;

      for (int32_t i = 0; i < height; i++)
      {
        if (textcolor != textbgcolor) fillRect(clip, cursor.x, pY, width * textsize, textsize, textbgcolor);

        for (int32_t k = 0; k < w; k++)
        {
          uint8_t line = pgm_read_byte((uint8_t *)flash_address + w * i + k);
          if (line) {
            if (textsize == 1) {
              int32_t pX = cursor.x + k * 8;
              if (line & 0x80) drawPixel(clip, pX, pY, textcolor);
              if (line & 0x40) drawPixel(clip, pX + 1, pY, textcolor);
              if (line & 0x20) drawPixel(clip, pX + 2, pY, textcolor);
              if (line & 0x10) drawPixel(clip, pX + 3, pY, textcolor);
              if (line & 0x08) drawPixel(clip, pX + 4, pY, textcolor);
              if (line & 0x04) drawPixel(clip, pX + 5, pY, textcolor);
              if (line & 0x02) drawPixel(clip, pX + 6, pY, textcolor);
              if (line & 0x01) drawPixel(clip, pX + 7, pY, textcolor);
            }
            else {
              int32_t pX = cursor.x + k * 8 * textsize;
              if (line & 0x80) fillRect(clip, pX, pY, textsize, textsize, textcolor);
              if (line & 0x40) fillRect(clip, pX + textsize, pY, textsize, textsize, textcolor);
              if (line & 0x20) fillRect(clip, pX + 2 * textsize, pY, textsize, textsize, textcolor);
              if (line & 0x10) fillRect(clip, pX + 3 * textsize, pY, textsize, textsize, textcolor);
              if (line & 0x08) fillRect(clip, pX + 4 * textsize, pY, textsize, textsize, textcolor);
              if (line & 0x04) fillRect(clip, pX + 5 * textsize, pY, textsize, textsize, textcolor);
              if (line & 0x02) fillRect(clip, pX + 6 * textsize, pY, textsize, textsize, textcolor);
              if (line & 0x01) fillRect(clip, pX + 7 * textsize, pY, textsize, textsize, textcolor);
            }
          }
        }
        pY += textsize;
      }
}


void TFT_CHAR::drawCharRLE_1(int32_t width, int32_t height, rgb_t textcolor, rgb_t textbgcolor, uint32_t flash_address)
{
        int32_t w = width;
        w *= height; // Now w is total number of pixels in the character

        // Maximum font size is equivalent to 180x180 pixels in area
        while (w > 0) {
          uint8_t line = pgm_read_byte((uint8_t *)flash_address++); // 8 bytes smaller when incrementing here
          if (line & 0x80) {
            line &= 0x7F;
            line++; w -= line;
            pushBlock16(textcolor, line);
          }
          else {
            line++; w -= line;
            pushBlock16(textbgcolor, line);
          }
        }
}

void TFT_CHAR::drawCharRLE_3(clip_t& clip, cursor_t& cursor, int32_t width, int32_t height, uint8_t textsize, rgb_t textcolor, rgb_t textbgcolor, uint32_t flash_address)
{
        int32_t w = width;
        w *= height; // Now w is total number of pixels in the character

        int32_t px = 0, py = 0;  // To hold character pixel coords
        int32_t tx = 0, ty = 0;  // To hold character TFT pixel coords
        int32_t pc = 0;          // Pixel count
        int32_t pl = 0;          // Pixel line length
        uint16_t pcol = 0;       // Pixel color
        bool     pf = true;      // Flag for plotting
        while (pc < w) {
          uint8_t line = pgm_read_byte((uint8_t *)flash_address);
          flash_address++;
          if (line & 0x80) { pcol = textcolor; line &= 0x7F; pf = true;}
          else { pcol = textbgcolor; if (textcolor == textbgcolor) pf = false;}
          line++;
          px = pc % width;
          tx = cursor.x + textsize * px;
          py = pc / width;
          ty = cursor.y + textsize * py;

          pl = 0;
          pc += line;
          while (line--) {
            pl++;
            if ((px+pl) >= width) {
              if (pf) fillRect(clip, tx, ty, pl * textsize, textsize, pcol);
              pl = 0;
              px = 0;
              tx = cursor.x;
              py ++;
              ty += textsize;
            }
          }
          if (pl && pf) fillRect(clip, tx, ty, pl * textsize, textsize, pcol);
        }
}


/***************************************************************************************
** Function name:           drawChar
** Description:             draw a Unicode glyph onto the screen
***************************************************************************************/
  // TODO: Rationalise with TFT_eSprite
  // Any UTF-8 decoding must be done before calling drawChar()
int16_t TFT_CHAR::drawChar(clip_t& clip, cursor_t& cursor, chr_font_t& font, uint16_t uniCode, rgb_t textcolor, rgb_t textbgcolor)
{
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
    if ((font.font > 2) && (font.font < 9)) {
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

    if (textcolor == textbgcolor || font.size != 1 || offclip) {
      //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
      inTransaction = true;
      drawCharFont2(clip, cursor, width, height, font.size, textcolor, textbgcolor, flash_address);
      inTransaction = lockTransaction;
      end_tft_write();
    }
    else { // Faster drawing of characters and background using block write
      begin_tft_write();
      setWindow(xd, yd, width, height);
      drawCharFont2Faster(width, height, textcolor, textbgcolor, flash_address);
      end_tft_write();
    }
  }

  #ifdef LOAD_RLE
  else
  #endif
#endif  //FONT2

#ifdef LOAD_RLE  //674 bytes of code
  // Font is not 2 and hence is RLE encoded
  {
    begin_tft_write();
    inTransaction = true;

    if (textcolor == textbgcolor && !offclip) {
      drawCharRLEfont(xd, yd, cursor.y, width, height, font.size, textcolor, flash_address);
    }
    else {
      // Text colour != background and font.size = 1 and character is within viewport area
      // so use faster drawing of characters and background using block write
      if (textcolor != textbgcolor && font.size == 1 && !offclip) {
        setWindow(xd, yd, width, height);
        drawCharRLE_1(width, height, textcolor, textbgcolor, flash_address);
      }
      else {
        drawCharRLE_3(clip, cursor, width, height, font.size, textcolor, textbgcolor, flash_address);
      }
    }
    inTransaction = lockTransaction;
    end_tft_write();
  }
  // End of RLE font rendering
#endif
  return width * font.size;    // x +
}


#ifdef SMOOTH_FONT
  #include "Extensions/Smooth_font.cpp"
#endif

/*
  Character drawing level

  Copyright (c) 2024, rspber (https://github.com/rspber)

  Based on: TFT_eSPI

  Originally notes below:
*/

/***************************************************
  Arduino TFT graphics library targeted at 32 bit
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
  cursor_y  = cursor_x  = last_cursor_x = bg_cursor_x = 0;
  textfont  = 1;
  textsize  = 1;
  textcolor   = bitmap_fg = 0xFFFF; // White
  textbgcolor = bitmap_bg = 0x0000; // Black
  padX        = 0;                  // No padding

  _fillbg    = false;   // Smooth font only at the moment, force text background fill

  isDigits   = false;   // No bounding box adjustment
  textwrapX  = true;    // Wrap text at end of line when using print stream
  textwrapY  = false;   // Wrap text at bottom of screen when using print stream
  textdatum = TL_DATUM; // Top Left text alignment is default
  fontsloaded = 0;

  _cp437    = true;     // Legacy GLCD font bug fix
  _utf8     = true;     // UTF8 decoding enabled

#if defined (FONT_FS_AVAILABLE) && defined (SMOOTH_FONT)
  fs_font  = true;     // Smooth font filing system or array (fs_font = false) flag
#endif

// Flags for which fonts are loaded
#ifdef LOAD_GLCD
  fontsloaded  = 0x0002; // Bit 1 set
#endif

#ifdef LOAD_FONT2
  fontsloaded |= 0x0004; // Bit 2 set
#endif

#ifdef LOAD_FONT4
  fontsloaded |= 0x0010; // Bit 4 set
#endif

#ifdef LOAD_FONT6
  fontsloaded |= 0x0040; // Bit 6 set
#endif

#ifdef LOAD_FONT7
  fontsloaded |= 0x0080; // Bit 7 set
#endif

#ifdef LOAD_FONT8
  fontsloaded |= 0x0100; // Bit 8 set
#endif

#ifdef LOAD_FONT8N
  fontsloaded |= 0x0200; // Bit 9 set
#endif

#ifdef SMOOTH_FONT
  fontsloaded |= 0x8000; // Bit 15 set
#endif
}


void TFT_CHAR::setCallback(getColorCallback getCol)
{
  getColor = getCol;
}


/***************************************************************************************
** Function name:           setCursor
** Description:             Set the text cursor x,y position
***************************************************************************************/
void TFT_CHAR::setCursor(int16_t x, int16_t y)
{
  cursor_x = x;
  cursor_y = y;
}


/***************************************************************************************
** Function name:           setCursor
** Description:             Set the text cursor x,y position and font
***************************************************************************************/
void TFT_CHAR::setCursor(int16_t x, int16_t y, uint8_t font)
{
  textfont = font;
  cursor_x = x;
  cursor_y = y;
}


/***************************************************************************************
** Function name:           getCursorX
** Description:             Get the text cursor x position
***************************************************************************************/
int16_t TFT_CHAR::getCursorX(void)
{
  return cursor_x;
}

/***************************************************************************************
** Function name:           getCursorY
** Description:             Get the text cursor y position
***************************************************************************************/
int16_t TFT_CHAR::getCursorY(void)
{
  return cursor_y;
}


/***************************************************************************************
** Function name:           setTextSize
** Description:             Set the text size multiplier
***************************************************************************************/
void TFT_CHAR::setTextSize(uint8_t s)
{
  if (s>7) s = 7; // Limit the maximum size multiplier so byte variables can be used for rendering
  textsize = (s > 0) ? s : 1; // Don't allow font size 0
}


/***************************************************************************************
** Function name:           setTextColor
** Description:             Set the font foreground colour (background is transparent)
***************************************************************************************/
void TFT_CHAR::setTextColor(uint16_t c)
{
  // For 'transparent' background, we'll set the bg
  // to the same as fg instead of using a flag
  textcolor = textbgcolor = c;
}


/***************************************************************************************
** Function name:           setTextColor
** Description:             Set the font foreground and background colour
***************************************************************************************/
// Smooth fonts use the background colour for anti-aliasing and by default the
// background is not filled. If bgfill = true, then a smooth font background fill will
// be used.
void TFT_CHAR::setTextColor(uint16_t c, uint16_t b, bool bgfill)
{
  textcolor   = c;
  textbgcolor = b;
  _fillbg     = bgfill;
}


/***************************************************************************************
** Function name:           setTextWrap
** Description:             Define if text should wrap at end of line
***************************************************************************************/
void TFT_CHAR::setTextWrap(bool wrapX, bool wrapY)
{
  textwrapX = wrapX;
  textwrapY = wrapY;
}


/***************************************************************************************
** Function name:           setTextDatum
** Description:             Set the text position reference datum
***************************************************************************************/
void TFT_CHAR::setTextDatum(uint8_t d)
{
  textdatum = d;
}


/***************************************************************************************
** Function name:           setTextPadding
** Description:             Define padding width (aids erasing old text and numbers)
***************************************************************************************/
void TFT_CHAR::setTextPadding(uint16_t x_width)
{
  padX = x_width;
}

/***************************************************************************************
** Function name:           setTextPadding
** Description:             Define padding width (aids erasing old text and numbers)
***************************************************************************************/
uint16_t TFT_CHAR::getTextPadding(void)
{
  return padX;
}

/***************************************************************************************
** Function name:           getTextDatum
** Description:             Return the text datum value (as used by setTextDatum())
***************************************************************************************/
uint8_t TFT_CHAR::getTextDatum(void)
{
  return textdatum;
}


/***************************************************************************************
** Function name:           textWidth
** Description:             Return the width in pixels of a string in a given font
***************************************************************************************/
int16_t TFT_CHAR::textWidth(const char *string, uint8_t font)
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
            if (*string || isDigits) str_width += gxAdvance[gNum];
            else str_width += (gdX[gNum] + gWidth[gNum]);
          }
          else str_width += gFont.spaceWidth + 1;
        }
      }
    }
    isDigits = false;
    return str_width;
  }
#endif

  if (font>1 && font<9) {
    char *widthtable = (char *)pgm_read_dword( &(fontdata[font].widthtbl ) ) - 32; //subtract the 32 outside the loop

    while (*string) {
      uniCode = *(string++);
      if (uniCode > 31 && uniCode < 128)
      str_width += pgm_read_byte( widthtable + uniCode); // Normally we need to subtract 32 from uniCode
      else str_width += pgm_read_byte( widthtable + 32); // Set illegal character = space width
    }

  }
  else {

#ifdef LOAD_GFXFF
    if(gfxFont) { // New font
      while (*string) {
        uniCode = decodeUTF8(*string++);
        if ((uniCode >= pgm_read_word(&gfxFont->first)) && (uniCode <= pgm_read_word(&gfxFont->last ))) {
          uniCode -= pgm_read_word(&gfxFont->first);
          GFXglyph *glyph  = &(((GFXglyph *)pgm_read_dword(&gfxFont->glyph))[uniCode]);
          // If this is not the  last character or is a digit then use xAdvance
          if (*string  || isDigits) str_width += pgm_read_byte(&glyph->xAdvance);
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
  isDigits = false;
  return str_width * textsize;
}


/***************************************************************************************
** Function name:           fontsLoaded
** Description:             return an encoded 16 bit value showing the fonts loaded
***************************************************************************************/
// Returns a value showing which fonts are loaded (bit N set =  Font N loaded)
uint16_t TFT_CHAR::fontsLoaded(void)
{
  return fontsloaded;
}


/***************************************************************************************
** Function name:           fontHeight
** Description:             return the height of a font (yAdvance for free fonts)
***************************************************************************************/
int16_t TFT_CHAR::fontHeight(int16_t font)
{
#ifdef SMOOTH_FONT
  if(fontLoaded) return gFont.yAdvance;
#endif

#ifdef LOAD_GFXFF
  if (font==1) {
    if(gfxFont) { // New font
      return pgm_read_byte(&gfxFont->yAdvance) * textsize;
    }
  }
#endif
  return pgm_read_byte( &fontdata[font].height ) * textsize;
}

/***************************************************************************************
** Function name:           drawChar
** Description:             draw a single character in the GLCD or GFXFF font
***************************************************************************************/
void TFT_CHAR::drawChar(clip_t& clip, int32_t x, int32_t y, uint16_t c, uint32_t color, uint32_t bg, uint8_t size)
{
  if (c < 32) return;
#ifdef LOAD_GLCD
//>>>>>>>>>>>>>>>>>>
  #ifdef LOAD_GFXFF
  if(!gfxFont) { // 'Classic' built-in font
  #endif
//>>>>>>>>>>>>>>>>>>

  int32_t xd = x + clip.xDatum;
  int32_t yd = y + clip.yDatum;

  if ((xd >= clip.x2)                 || // Clip right
     ( yd >= clip.y2)                 || // Clip bottom
     ((xd + 6 * size - 1) < clip.x1)  || // Clip left
     ((yd + 8 * size - 1) < clip.y1))    // Clip top
    return;

  bool fillbg = (bg != color);
  bool offclip = xd < clip.x1 || xd + 6  * textsize >= clip.x2 || yd < clip.y1 || yd + 8 * textsize >= clip.y2;

  if ((size==1) && fillbg && !offclip) {
    uint8_t column[6];
    uint8_t mask = 0x1;
    begin_tft_write();

    setWindow(xd, yd, 6, 8);

    drawCharDefault(font + (c * 5), color, bg);

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
        line = pgm_read_byte(font + (c * 5) + i);

      if (size == 1 && !fillbg) { // default size
        for (int8_t j = 0; j < 8; j++) {
          if (line & 0x1) drawPixel(clip, x + i, y + j, color);
          line >>= 1;
        }
      }
      else {  // big size or clipped
        for (int8_t j = 0; j < 8; j++) {
          if (line & 0x1) fillRect(clip, x + (i * size), y + (j * size), size, size, color);
          else if (fillbg) fillRect(clip, x + i * size, y + j * size, size, size, bg);
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
    if ((c >= pgm_read_word(&gfxFont->first)) && (c <= pgm_read_word(&gfxFont->last ))) {
      //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
      inTransaction = true;
//>>>>>>>>>>>>>>>>>>>>>>>>>>>

      c -= pgm_read_word(&gfxFont->first);
      GFXglyph *glyph  = &(((GFXglyph *)pgm_read_dword(&gfxFont->glyph))[c]);
      uint8_t  *bitmap = (uint8_t *)pgm_read_dword(&gfxFont->bitmap);

      uint32_t bo = pgm_read_word(&glyph->bitmapOffset);
      uint8_t  w  = pgm_read_byte(&glyph->width),
               h  = pgm_read_byte(&glyph->height);
               //xa = pgm_read_byte(&glyph->xAdvance);
      int8_t   xo = pgm_read_byte(&glyph->xOffset),
               yo = pgm_read_byte(&glyph->yOffset);
      uint8_t  xx, yy, bits=0, bit=0;
      int16_t  xo16 = 0, yo16 = 0;

      if(size > 1) {
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
              if(size == 1) drawFastHLine(clip, x+xo+xx-hpc, y+yo+yy, hpc, color);
              else fillRect(clip, x+(xo16+xx-hpc)*size, y+(yo16+yy)*size, size*hpc, size, color);
              hpc=0;
            }
          }
          bit >>= 1;
        }
        // Draw pixels for this line as we are about to increment yy
        if (hpc) {
          if(size == 1) drawFastHLine(clip, x+xo+xx-hpc, y+yo+yy, hpc, color);
          else fillRect(clip, x+(xo16+xx-hpc)*size, y+(yo16+yy)*size, size*hpc, size, color);
          hpc=0;
        }
      }

      inTransaction = lockTransaction;
      end_tft_write();              // Does nothing if Sprite class uses this function
    }
#endif

#ifdef LOAD_GLCD
  #ifdef LOAD_GFXFF
  } // End classic vs custom font
  #endif
#else
  #ifndef LOAD_GFXFF
    // Avoid warnings if fonts are disabled
    x = x;
    y = y;
    color = color;
    bg = bg;
    size = size;
  #endif
#endif

}


/***************************************************************************************
** Function name:           decodeUTF8
** Description:             Serial UTF-8 decoder with fall-back to extended ASCII
*************************************************************************************x*/
uint16_t TFT_CHAR::decodeUTF8(uint8_t c)
{
  if (!_utf8) return c;

  // 7 bit Unicode Code Point
  if ((c & 0x80) == 0x00) {
    decoderState = 0;
    return c;
  }

  if (decoderState == 0) {
    // 11 bit Unicode Code Point
    if ((c & 0xE0) == 0xC0) {
      decoderBuffer = ((c & 0x1F)<<6);
      decoderState = 1;
      return 0;
    }
    // 16 bit Unicode Code Point
    if ((c & 0xF0) == 0xE0) {
      decoderBuffer = ((c & 0x0F)<<12);
      decoderState = 2;
      return 0;
    }
    // 21 bit Unicode  Code Point not supported so fall-back to extended ASCII
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

  // 7 bit Unicode
  if ((c & 0x80) == 0x00) return c;

  // 11 bit Unicode
  if (((c & 0xE0) == 0xC0) && (remaining > 1))
    return ((c & 0x1F)<<6) | (buf[(*index)++]&0x3F);

  // 16 bit Unicode
  if (((c & 0xF0) == 0xE0) && (remaining > 2)) {
    c = ((c & 0x0F)<<12) | ((buf[(*index)++]&0x3F)<<6);
    return  c | ((buf[(*index)++]&0x3F));
  }

  // 21 bit Unicode not supported so fall-back to extended ASCII
  // if ((c & 0xF8) == 0xF0) return c;

  return c; // fall-back to extended ASCII
}


/***************************************************************************************
** Function name:           write
** Description:             draw characters piped through serial stream
***************************************************************************************/
size_t TFT_CHAR::write(wh_clip_t& clip, uint8_t utf8)
{
  uint16_t uniCode = decodeUTF8(utf8);

  if (!uniCode) return 1;

  if (utf8 == '\r') return 1;

#ifdef SMOOTH_FONT
  if(fontLoaded) {
    if (uniCode < 32 && utf8 != '\n') return 1;

    drawGlyph(clip, uniCode);

    return 1;
  }
#endif

  if (uniCode == '\n') uniCode+=22; // Make it a valid space character to stop errors
  else if (uniCode < 32) return 1;

  uint16_t cwidth = 0;
  uint16_t cheight = 0;

//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv DEBUG vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
  //Serial.print((uint8_t) uniCode); // Debug line sends all printed TFT text to serial port
  //Serial.println(uniCode, HEX); // Debug line sends all printed TFT text to serial port
  //delay(5);                     // Debug optional wait for serial port to flush through
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ DEBUG ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#ifdef LOAD_GFXFF
  if(!gfxFont) {
#endif
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#ifdef LOAD_FONT2
  if (textfont == 2) {
    if (uniCode > 127) return 1;

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
    if ((textfont>2) && (textfont<9)) {
      if (uniCode > 127) return 1;
      // Uses the fontinfo struct array to avoid lots of 'if' or 'switch' statements
      cwidth = pgm_read_byte( (uint8_t *)pgm_read_dword( &(fontdata[textfont].widthtbl ) ) + uniCode-32 );
      cheight= pgm_read_byte( &fontdata[textfont].height );
    }
  }
#endif

#ifdef LOAD_GLCD
  if (textfont==1) {
      cwidth =  6;
      cheight = 8;
  }
#else
  if (textfont==1) return 1;
#endif

  cheight = cheight * textsize;

  if (utf8 == '\n') {
    cursor_y += cheight;
    cursor_x  = 0;
  }
  else {
    if (textwrapX && (cursor_x + cwidth * textsize > clip.width)) {
      cursor_y += cheight;
      cursor_x = 0;
    }
    if (textwrapY && (cursor_y >= (int32_t) clip.height)) cursor_y = 0;
    cursor_x += drawChar(clip, uniCode, cursor_x, cursor_y, textfont);
  }

//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
#ifdef LOAD_GFXFF
  } // Custom GFX font
  else {
    if(utf8 == '\n') {
      cursor_x  = 0;
      cursor_y += (int16_t)textsize * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
    } else {
      if (uniCode > pgm_read_word(&gfxFont->last )) return 1;
      if (uniCode < pgm_read_word(&gfxFont->first)) return 1;

      uint16_t   c2    = uniCode - pgm_read_word(&gfxFont->first);
      GFXglyph *glyph = &(((GFXglyph *)pgm_read_dword(&gfxFont->glyph))[c2]);
      uint8_t   w     = pgm_read_byte(&glyph->width),
                h     = pgm_read_byte(&glyph->height);
      if((w > 0) && (h > 0)) { // Is there an associated bitmap?
        int16_t xo = (int8_t)pgm_read_byte(&glyph->xOffset);
        if(textwrapX && ((cursor_x + textsize * (xo + w)) > clip.width)) {
          // Drawing character would go off right edge; wrap to new line
          cursor_x  = 0;
          cursor_y += (int16_t)textsize * (uint8_t)pgm_read_byte(&gfxFont->yAdvance);
        }
        if (textwrapY && (cursor_y >= (int32_t) clip.height)) cursor_y = 0;
        drawChar(clip, cursor_x, cursor_y, uniCode, textcolor, textbgcolor, textsize);
      }
      cursor_x += pgm_read_byte(&glyph->xAdvance) * (int16_t)textsize;
    }
  }
#endif // LOAD_GFXFF
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

  return 1;
}


/***************************************************************************************
** Function name:           drawChar
** Description:             draw a Unicode glyph onto the screen
***************************************************************************************/
  // TODO: Rationalise with TFT_eSprite
  // Any UTF-8 decoding must be done before calling drawChar()
int16_t TFT_CHAR::drawChar(clip_t& clip, uint16_t uniCode, int32_t x, int32_t y, uint8_t font)
{
  if (!uniCode) return 0;

  if (font==1) {
#ifdef LOAD_GLCD
  #ifndef LOAD_GFXFF
    drawChar(clip, x, y, uniCode, textcolor, textbgcolor, textsize);
    return 6 * textsize;
  #endif
#else
  #ifndef LOAD_GFXFF
    return 0;
  #endif
#endif

#ifdef LOAD_GFXFF
    drawChar(clip, x, y, uniCode, textcolor, textbgcolor, textsize);
    if(!gfxFont) { // 'Classic' built-in font
    #ifdef LOAD_GLCD
      return 6 * textsize;
    #else
      return 0;
    #endif
    }
    else {
      if((uniCode >= pgm_read_word(&gfxFont->first)) && (uniCode <= pgm_read_word(&gfxFont->last) )) {
        uint16_t   c2    = uniCode - pgm_read_word(&gfxFont->first);
        GFXglyph *glyph = &(((GFXglyph *)pgm_read_dword(&gfxFont->glyph))[c2]);
        return pgm_read_byte(&glyph->xAdvance) * textsize;
      }
      else {
        return 0;
      }
    }
#endif
  }

  if ((font>1) && (font<9) && ((uniCode < 32) || (uniCode > 127))) return 0;

  int32_t width  = 0;
  int32_t height = 0;
  uint32_t flash_address = 0;
  uniCode -= 32;

#ifdef LOAD_FONT2
  if (font == 2) {
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
    if ((font>2) && (font<9)) {
      flash_address = pgm_read_dword( (const void*)(pgm_read_dword( &(fontdata[font].chartbl ) ) + uniCode*sizeof(void *)) );
      width = pgm_read_byte( (uint8_t *)pgm_read_dword( &(fontdata[font].widthtbl ) ) + uniCode );
      height= pgm_read_byte( &fontdata[font].height );
    }
  }
#endif

  int32_t xd = x + clip.xDatum;
  int32_t yd = y + clip.yDatum;

  if ((xd + width * textsize < clip.x1 || xd >= clip.x2) && (yd + height * textsize < clip.y1 || yd >= clip.y2)) return width * textsize ;

  bool offclip = xd < clip.x1 || xd + width  * textsize >= clip.x2 || yd < clip.y1 || yd + height * textsize >= clip.y2;

#ifdef LOAD_FONT2 // chop out code if we do not need it
  if (font == 2) {

    if (textcolor == textbgcolor || textsize != 1 || offclip) {
      //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
      inTransaction = true;

      int32_t pX      = 0;
      int32_t pY      = y;
      int32_t w = width;
      w = w + 6; // Should be + 7 but we need to compensate for width increment
      w = w / 8;
      uint8_t line = 0;

      for (int32_t i = 0; i < height; i++) {
        if (textcolor != textbgcolor) fillRect(clip, x, pY, width * textsize, textsize, textbgcolor);

        for (int32_t k = 0; k < w; k++) {
          line = pgm_read_byte((uint8_t *)flash_address + w * i + k);
          if (line) {
            if (textsize == 1) {
              pX = x + k * 8;
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
              pX = x + k * 8 * textsize;
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

      inTransaction = lockTransaction;
      end_tft_write();
    }
    else { // Faster drawing of characters and background using block write

      begin_tft_write();

      setWindow(xd, yd, width, height);
      drawCharFaster(width, height, textcolor, textbgcolor, flash_address);

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

      drawRLEfont(xd, yd, y, width, height, textcolor, textsize, flash_address);
    }
    else {
      // Text colour != background and textsize = 1 and character is within viewport area
      // so use faster drawing of characters and background using block write
      if (textcolor != textbgcolor && textsize == 1 && !offclip)
      {
        setWindow(xd, yd, width, height);

        int32_t w = width;
        w *= height; // Now w is total number of pixels in the character

        // Maximum font size is equivalent to 180x180 pixels in area
        while (w > 0) {
          uint8_t line = pgm_read_byte((uint8_t *)flash_address++); // 8 bytes smaller when incrementing here
          if (line & 0x80) {
            line &= 0x7F;
            line++; w -= line;
            pushBlock(textcolor,line);
          }
          else {
            line++; w -= line;
            pushBlock(textbgcolor,line);
          }
        }
      }
      else
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
          tx = x + textsize * px;
          py = pc / width;
          ty = y + textsize * py;

          pl = 0;
          pc += line;
          while (line--) {
            pl++;
            if ((px+pl) >= width) {
              if (pf) fillRect(clip, tx, ty, pl * textsize, textsize, pcol);
              pl = 0;
              px = 0;
              tx = x;
              py ++;
              ty += textsize;
            }
          }
          if (pl && pf) fillRect(clip, tx, ty, pl * textsize, textsize, pcol);
        }
      }
    }
    inTransaction = lockTransaction;
    end_tft_write();
  }
  // End of RLE font rendering
#endif

#if !defined (LOAD_FONT2) && !defined (LOAD_RLE)
  // Stop warnings
  flash_address = flash_address;
  w = w;
  pX = pX;
  pY = pY;
  line = line;
  clip = clip;
#endif

  return width * textsize;    // x +
}


/***************************************************************************************
** Function name:           drawString (with or without user defined font)
** Description :            draw string with padding if it is defined
***************************************************************************************/
// With font number. Note: font number is over-ridden if a smooth font is loaded
int16_t TFT_CHAR::drawString(wh_clip_t& clip, const char *string, int32_t poX, int32_t poY, uint8_t font)
{
  int16_t sumX = 0;
  uint8_t padding = 1, baseline = 0;
  uint16_t cwidth = textWidth(string, font); // Find the pixel width of the string in the font
  uint16_t cheight = 8 * textsize;

#ifdef LOAD_GFXFF
  #ifdef SMOOTH_FONT
    bool freeFont = (font == 1 && gfxFont && !fontLoaded);
  #else
    bool freeFont = (font == 1 && gfxFont);
  #endif

  if (freeFont) {
    cheight = glyph_ab * textsize;
    poY += cheight; // Adjust for baseline datum of free fonts
    baseline = cheight;
    padding =101; // Different padding method used for Free Fonts

    // We need to make an adjustment for the bottom of the string (eg 'y' character)
    if ((textdatum == BL_DATUM) || (textdatum == BC_DATUM) || (textdatum == BR_DATUM)) {
      cheight += glyph_bb * textsize;
    }
  }
#endif


  // If it is not font 1 (GLCD or free font) get the baseline and pixel height of the font
#ifdef SMOOTH_FONT
  if(fontLoaded) {
    baseline = gFont.maxAscent;
    cheight  = fontHeight(textfont);
  }
  else
#endif
  if (font!=1) {
    baseline = pgm_read_byte( &fontdata[font].baseline ) * textsize;
    cheight = fontHeight(font);
  }

  if (textdatum || padX) {

    switch(textdatum) {
      case TC_DATUM:
        poX -= cwidth/2;
        padding += 1;
        break;
      case TR_DATUM:
        poX -= cwidth;
        padding += 2;
        break;
      case ML_DATUM:
        poY -= cheight/2;
        //padding += 0;
        break;
      case MC_DATUM:
        poX -= cwidth/2;
        poY -= cheight/2;
        padding += 1;
        break;
      case MR_DATUM:
        poX -= cwidth;
        poY -= cheight/2;
        padding += 2;
        break;
      case BL_DATUM:
        poY -= cheight;
        //padding += 0;
        break;
      case BC_DATUM:
        poX -= cwidth/2;
        poY -= cheight;
        padding += 1;
        break;
      case BR_DATUM:
        poX -= cwidth;
        poY -= cheight;
        padding += 2;
        break;
      case L_BASELINE:
        poY -= baseline;
        //padding += 0;
        break;
      case C_BASELINE:
        poX -= cwidth/2;
        poY -= baseline;
        padding += 1;
        break;
      case R_BASELINE:
        poX -= cwidth;
        poY -= baseline;
        padding += 2;
        break;
    }
  }


  int8_t xo = 0;
#ifdef LOAD_GFXFF
  if (freeFont && (textcolor!=textbgcolor)) {
      cheight = (glyph_ab + glyph_bb) * textsize;
      // Get the offset for the first character only to allow for negative offsets
      uint16_t c2 = 0;
      uint16_t len = strlen(string);
      uint16_t n = 0;

      while (n < len && c2 == 0) c2 = decodeUTF8((uint8_t*)string, &n, len - n);

      if((c2 >= pgm_read_word(&gfxFont->first)) && (c2 <= pgm_read_word(&gfxFont->last) )) {
        c2 -= pgm_read_word(&gfxFont->first);
        GFXglyph *glyph = &(((GFXglyph *)pgm_read_dword(&gfxFont->glyph))[c2]);
        xo = pgm_read_byte(&glyph->xOffset) * textsize;
        // Adjust for negative xOffset
        if (xo > 0) xo = 0;
        else cwidth -= xo;
        // Add 1 pixel of padding all round
        //cheight +=2;
        //fillRect(poX+xo-1, poY - 1 - glyph_ab * textsize, cwidth+2, cheight, textbgcolor);
        fillRect(clip, poX+xo, poY - glyph_ab * textsize, cwidth, cheight, textbgcolor);
      }
      padding -=100;
    }
#endif

  uint16_t len = strlen(string);
  uint16_t n = 0;

#ifdef SMOOTH_FONT
  if(fontLoaded) {
    setCursor(poX, poY);

    bool fillbg = _fillbg;
    // If padding is requested then fill the text background
    if (padX && !_fillbg) _fillbg = true;

    while (n < len) {
      uint16_t uniCode = decodeUTF8((uint8_t*)string, &n, len - n);
      drawGlyph(clip, uniCode);
    }
    _fillbg = fillbg; // restore state
    sumX += cwidth;
    //fontFile.close();
  }
  else
#endif
  {
    while (n < len) {
      uint16_t uniCode = decodeUTF8((uint8_t*)string, &n, len - n);
      sumX += drawChar(clip, uniCode, poX+sumX, poY, font);
    }
  }

//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv DEBUG vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// Switch on debugging for the padding areas
//#define PADDING_DEBUG

#ifndef PADDING_DEBUG
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ DEBUG ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

  if((padX>cwidth) && (textcolor!=textbgcolor)) {
    int16_t padXc = poX+cwidth+xo;
#ifdef LOAD_GFXFF
    if (freeFont) {
      poX +=xo; // Adjust for negative offset start character
      poY -= glyph_ab * textsize;
      sumX += poX;
    }
#endif
    switch(padding) {
      case 1:
        fillRect(clip, padXc,poY,padX-cwidth,cheight, textbgcolor);
        break;
      case 2:
        fillRect(clip, padXc,poY,(padX-cwidth)>>1,cheight, textbgcolor);
        padXc = poX - ((padX-cwidth)>>1);
        fillRect(clip, padXc,poY,(padX-cwidth)>>1,cheight, textbgcolor);
        break;
      case 3:
        if (padXc>padX) padXc = padX;
        fillRect(clip, poX + cwidth - padXc,poY,padXc-cwidth,cheight, textbgcolor);
        break;
    }
  }


#else

//vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv DEBUG vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
// This is debug code to show text (green box) and blanked (white box) areas
// It shows that the padding areas are being correctly sized and positioned

  if((padX>sumX) && (textcolor!=textbgcolor)) {
    int16_t padXc = poX+sumX; // Maximum left side padding
#ifdef LOAD_GFXFF
    if ((font == 1) && (gfxFont)) poY -= glyph_ab;
#endif
    drawRect(poX,poY,sumX,cheight, TFT_GREEN);
    switch(padding) {
      case 1:
        drawRect(clip, padXc,poY,padX-sumX,cheight, TFT_WHITE);
        break;
      case 2:
        drawRect(clip, padXc,poY,(padX-sumX)>>1, cheight, TFT_WHITE);
        padXc = (padX-sumX)>>1;
        drawRect(clip, poX - padXc,poY,(padX-sumX)>>1,cheight, TFT_WHITE);
        break;
      case 3:
        if (padXc>padX) padXc = padX;
        drawRect(clip, poX + sumX - padXc,poY,padXc-sumX,cheight, TFT_WHITE);
        break;
    }
  }
#endif
//^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ DEBUG ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

return sumX;
}


/***************************************************************************************
** Function name:           drawCentreString (deprecated, use setTextDatum())
** Descriptions:            draw string centred on dX
***************************************************************************************/
int16_t TFT_CHAR::drawCentreString(wh_clip_t& clip, const char *string, int32_t dX, int32_t poY, uint8_t font)
{
  uint8_t tempdatum = textdatum;
  int32_t sumX = 0;
  textdatum = TC_DATUM;
  sumX = drawString(clip, string, dX, poY, font);
  textdatum = tempdatum;
  return sumX;
}


/***************************************************************************************
** Function name:           drawRightString (deprecated, use setTextDatum())
** Descriptions:            draw string right justified to dX
***************************************************************************************/
int16_t TFT_CHAR::drawRightString(wh_clip_t& clip, const char *string, int32_t dX, int32_t poY, uint8_t font)
{
  uint8_t tempdatum = textdatum;
  int16_t sumX = 0;
  textdatum = TR_DATUM;
  sumX = drawString(clip, string, dX, poY, font);
  textdatum = tempdatum;
  return sumX;
}


/***************************************************************************************
** Function name:           drawNumber
** Description:             draw a long integer
***************************************************************************************/
int16_t TFT_CHAR::drawNumber(wh_clip_t& clip, long long_num, int32_t poX, int32_t poY, uint8_t font)
{
  isDigits = true; // Eliminate jiggle in monospaced fonts
  char str[12];
  ltoa(long_num, str, 10);
  return drawString(clip, str, poX, poY, font);
}


/***************************************************************************************
** Function name:           drawFloat
** Descriptions:            drawFloat, prints 7 non zero digits maximum
***************************************************************************************/
// Assemble and print a string, this permits alignment relative to a datum
// looks complicated but much more compact and actually faster than using print class
int16_t TFT_CHAR::drawFloat(wh_clip_t& clip, float floatNumber, uint8_t dp, int32_t poX, int32_t poY, uint8_t font)
{
  isDigits = true;
  char str[14];               // Array to contain decimal string
  uint8_t ptr = 0;            // Initialise pointer for array
  int8_t  digits = 1;         // Count the digits to avoid array overflow
  float rounding = 0.5;       // Round up down delta
  bool negative = false;

  if (dp > 7) dp = 7; // Limit the size of decimal portion

  // Adjust the rounding value
  for (uint8_t i = 0; i < dp; ++i) rounding /= 10.0;

  if (floatNumber < -rounding) {   // add sign, avoid adding - sign to 0.0!
    str[ptr++] = '-'; // Negative number
    str[ptr] = 0; // Put a null in the array as a precaution
    digits = 0;   // Set digits to 0 to compensate so pointer value can be used later
    floatNumber = -floatNumber; // Make positive
    negative = true;
  }

  floatNumber += rounding; // Round up or down

  if (dp == 0) {
    if (negative) floatNumber = -floatNumber;
    return drawNumber(clip, (long)floatNumber, poX, poY, font);
  }

  // For error put ... in string and return (all TFT_eSPI library fonts contain . character)
  if (floatNumber >= 2147483647) {
    strcpy(str, "...");
    return drawString(clip, str, poX, poY, font);
  }
  // No chance of overflow from here on

  // Get integer part
  uint32_t temp = (uint32_t)floatNumber;

  // Put integer part into array
  ltoa(temp, str + ptr, 10);

  // Find out where the null is to get the digit count loaded
  while ((uint8_t)str[ptr] != 0) ptr++; // Move the pointer along
  digits += ptr;                  // Count the digits

  str[ptr++] = '.'; // Add decimal point
  str[ptr] = '0';   // Add a dummy zero
  str[ptr + 1] = 0; // Add a null but don't increment pointer so it can be overwritten

  // Get the decimal portion
  floatNumber = floatNumber - temp;

  // Get decimal digits one by one and put in array
  // Limit digit count so we don't get a false sense of resolution
  uint8_t i = 0;
  while ((i < dp) && (digits < 9)) { // while (i < dp) for no limit but array size must be increased
    i++;
    floatNumber *= 10;       // for the next decimal
    temp = floatNumber;      // get the decimal
    ltoa(temp, str + ptr, 10);
    ptr++; digits++;         // Increment pointer and digits count
    floatNumber -= temp;     // Remove that digit
  }

  // Finally we can plot the string and return pixel length
  return drawString(clip, str, poX, poY, font);
}


/***************************************************************************************
** Function name:           setFreeFont
** Descriptions:            Sets the GFX free font to use
***************************************************************************************/

#ifdef LOAD_GFXFF

void TFT_CHAR::setFreeFont(const GFXfont *f)
{
  if (f == nullptr) { // Fix issue #400 (ESP32 crash)
    setTextFont(1); // Use GLCD font
    return;
  }

  textfont = 1;
  gfxFont = (GFXfont *)f;

  glyph_ab = 0;
  glyph_bb = 0;
  uint16_t numChars = pgm_read_word(&gfxFont->last) - pgm_read_word(&gfxFont->first);

  // Find the biggest above and below baseline offsets
  for (uint16_t c = 0; c < numChars; c++) {
    GFXglyph *glyph1  = &(((GFXglyph *)pgm_read_dword(&gfxFont->glyph))[c]);
    int8_t ab = -pgm_read_byte(&glyph1->yOffset);
    if (ab > glyph_ab) glyph_ab = ab;
    int8_t bb = pgm_read_byte(&glyph1->height) - ab;
    if (bb > glyph_bb) glyph_bb = bb;
  }
}


/***************************************************************************************
** Function name:           setTextFont
** Description:             Set the font for the print stream
***************************************************************************************/
void TFT_CHAR::setTextFont(uint8_t f)
{
  textfont = (f > 0) ? f : 1; // Don't allow font 0
  gfxFont = NULL;
}

#else


/***************************************************************************************
** Function name:           setFreeFont
** Descriptions:            Sets the GFX free font to use
***************************************************************************************/

// Alternative to setTextFont() so we don't need two different named functions
void TFT_CHAR::setFreeFont(uint8_t font)
{
  setTextFont(font);
}


/***************************************************************************************
** Function name:           setTextFont
** Description:             Set the font for the print stream
***************************************************************************************/
void TFT_CHAR::setTextFont(uint8_t f)
{
  textfont = (f > 0) ? f : 1; // Don't allow font 0
}
#endif

#ifdef SMOOTH_FONT
  #include "Extensions/Smooth_font.cpp"
#endif

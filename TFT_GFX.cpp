/*
  Graphics level

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

#include "TFT_eSPI.h"

/***************************************************************************************
** Function name:           TFT_eSPI
** Description:             Constructor , we must use hardware SPI pins
***************************************************************************************/
TFT_GFX::TFT_GFX() : TFT_eeSPI()
{
}

/***************************************************************************************
** Function name:           push rectangle
** Description:             push 565 pixel colours into a defined area
***************************************************************************************/
void TFT_GFX::pushRect(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data)
{
  pushImage(clip, x, y, w, h, false, data);
}


/***************************************************************************************
** Function name:           pushImage
** Description:             plot 16 bit colour sprite or image onto TFT
***************************************************************************************/
void TFT_GFX::pushImage(clip_t& clip, int32_t x0, int32_t y0, int32_t w, int32_t h, bool swapBytes, uint16_t *data)
{
  block_t z;
  if (!clip.check_block(z, x0, y0, w, h)) return;

  begin_tft_write();
  inTransaction = true;

  setWindow(z.x, z.y, z.dw, z.dh);

  data += z.dx + z.dy * w;

  // Check if whole image can be pushed
  if (z.dw == w) pushPixels(data, z.dw * z.dh, swapBytes);
  else {
    // Push line segments to crop image
    while (z.dh--)
    {
      pushPixels(data, z.dw, swapBytes);
      data += w;
    }
  }

  inTransaction = lockTransaction;
  end_tft_write();
}


/***************************************************************************************
** Function name:           pushImage
** Description:             plot 16 bit sprite or image with 1 colour being transparent
***************************************************************************************/
void TFT_GFX::pushImage(clip_t& clip, int32_t x0, int32_t y0, int32_t w, int32_t h, bool swapBytes, uint16_t *data, uint16_t transp)
{
  block_t z;
  if (!clip.check_block(z, x0, y0, w, h)) return;

  begin_tft_write();
  inTransaction = true;

  data += z.dx + z.dy * w;


  uint16_t  lineBuf[z.dw]; // Use buffer to minimise setWindow call count

  // The little endian transp color must be byte swapped if the image is big endian
  if (!swapBytes) transp = transp >> 8 | transp << 8;

  while (z.dh--)
  {
    int32_t len = z.dw;
    uint16_t* ptr = data;
    int32_t px = z.x, sx = z.x;
    bool move = true;
    uint16_t np = 0;

    while (len--)
    {
      if (transp != *ptr)
      {
        if (move) { move = false; sx = px; }
        lineBuf[np] = *ptr;
        np++;
      }
      else
      {
        move = true;
        if (np)
        {
          setWindow(sx, z.y, np, 1);
          pushPixels((uint16_t*)lineBuf, np, swapBytes);
          np = 0;
        }
      }
      px++;
      ptr++;
    }
    if (np) { setWindow(sx, z.y, np, 1); pushPixels((uint16_t*)lineBuf, np, swapBytes); }

    z.y++;
    data += w;
  }

  inTransaction = lockTransaction;
  end_tft_write();
}


/***************************************************************************************
** Function name:           pushImage - for FLASH (PROGMEM) stored images
** Description:             plot 16 bit image
***************************************************************************************/
void TFT_GFX::pushImage(clip_t& clip, int32_t x0, int32_t y0, int32_t w, int32_t h, bool swapBytes, const uint16_t *data)
{
  // Requires 32 bit aligned access, so use PROGMEM 16 bit word functions
  block_t z;
  if (!clip.check_block(z, x0, y0, w, h)) return;

  begin_tft_write();
  inTransaction = true;

  data += z.dx + z.dy * w;

  uint16_t  buffer[z.dw];

  setWindow(z.x, z.y, z.dw, z.dh);

  // Fill and send line buffers to TFT
  for (int32_t i = 0; i < z.dh; i++) {
    for (int32_t j = 0; j < z.dw; j++) {
      buffer[j] = pgm_read_word(&data[i * w + j]);
    }
    pushPixels(buffer, z.dw, swapBytes);
  }

  inTransaction = lockTransaction;
  end_tft_write();
}

/***************************************************************************************
** Function name:           pushImage - for FLASH (PROGMEM) stored images
** Description:             plot 16 bit image with 1 colour being transparent
***************************************************************************************/
void TFT_GFX::pushImage(clip_t& clip, int32_t x0, int32_t y0, int32_t w, int32_t h, bool swapBytes, const uint16_t *data, uint16_t transp)
{
  // Requires 32 bit aligned access, so use PROGMEM 16 bit word functions
  block_t z;
  if (!clip.check_block(z, x0, y0, w, h)) return;

  begin_tft_write();
  inTransaction = true;

  data += z.dx + z.dy * w;


  uint16_t  lineBuf[z.dw];

  // The little endian transp color must be byte swapped if the image is big endian
  if (!swapBytes) transp = transp >> 8 | transp << 8;

  while (z.dh--) {
    int32_t len = z.dw;
    uint16_t* ptr = (uint16_t*)data;
    int32_t px = z.x, sx = z.x;
    bool move = true;

    uint16_t np = 0;

    while (len--) {
      uint16_t color = pgm_read_word(ptr);
      if (transp != color) {
        if (move) { move = false; sx = px; }
        lineBuf[np] = color;
        np++;
      }
      else {
        move = true;
        if (np) {
          setWindow(sx, z.y, np, 1);
          pushPixels(lineBuf, np, swapBytes);
          np = 0;
        }
      }
      px++;
      ptr++;
    }
    if (np) { setWindow(sx, z.y, np, 1); pushPixels(lineBuf, np, swapBytes); }

    z.y++;
    data += w;
  }

  inTransaction = lockTransaction;
  end_tft_write();
}

/***************************************************************************************
** Function name:           pushImage
** Description:             plot 8 bit or 4 bit or 1 bit image or sprite using a line buffer
***************************************************************************************/
void TFT_GFX::pushImage(clip_t& clip, int32_t x0, int32_t y0, int32_t w, int32_t h, uint16_t bitmap_fg, uint16_t bitmap_bg, const uint8_t *data, bool bpp8, uint16_t *cmap)
{
  block_t z;
  if (!clip.check_block(z, x0, y0, w, h)) return;

  begin_tft_write();
  inTransaction = true;

  setWindow(z.x, z.y, z.dw, z.dh); // Sets CS low and sent RAMWR

  // Line buffer makes plotting faster
  uint16_t  lineBuf[z.dw];

  if (bpp8)
  {
    uint8_t  blue[] = {0, 11, 21, 31}; // blue 2 to 5 bit colour lookup table

    uint32_t _lastColor = -1; // Set to illegal value

    // Used to store last shifted colour
    uint8_t msbColor = 0;
    uint8_t lsbColor = 0;

    data += z.dx + z.dy * w;
    while (z.dh--) {
      uint32_t len = z.dw;
      uint8_t* ptr = (uint8_t*)data;
      uint8_t* linePtr = (uint8_t*)lineBuf;

      while(len--) {
        uint32_t color = pgm_read_byte(ptr++);

        // Shifts are slow so check if colour has changed first
        if (color != _lastColor) {
          //          =====Green=====     ===============Red==============
          msbColor = (color & 0x1C)>>2 | (color & 0xC0)>>3 | (color & 0xE0);
          //          =====Green=====    =======Blue======
          lsbColor = (color & 0x1C)<<3 | blue[color & 0x03];
          _lastColor = color;
        }

       *linePtr++ = msbColor;
       *linePtr++ = lsbColor;
      }

      pushPixels(lineBuf, z.dw, false);

      data += w;
    }
  }
  else if (cmap != nullptr) // Must be 4bpp
  {
    w = (w+1) & 0xFFFE;   // if this is a sprite, w will already be even; this does no harm.
    bool splitFirst = (z.dx & 0x01) != 0; // split first means we have to push a single px from the left of the sprite / image

    if (splitFirst) {
      data += ((z.dx - 1 + z.dy * w) >> 1);
    }
    else {
      data += ((z.dx + z.dy * w) >> 1);
    }

    while (z.dh--) {
      uint32_t len = z.dw;
      uint8_t * ptr = (uint8_t*)data;
      uint16_t *linePtr = lineBuf;
      uint8_t colors; // two colors in one byte
      uint16_t index;

      if (splitFirst) {
        colors = pgm_read_byte(ptr);
        index = (colors & 0x0F);
        *linePtr++ = cmap[index];
        len--;
        ptr++;
      }

      while (len--)
      {
        colors = pgm_read_byte(ptr);
        index = ((colors & 0xF0) >> 4) & 0x0F;
        *linePtr++ = cmap[index];

        if (len--)
        {
          index = colors & 0x0F;
          *linePtr++ = cmap[index];
        } else {
          break;  // nothing to do here
        }

        ptr++;
      }

      pushPixels(lineBuf, z.dw, true);
      data += (w >> 1);
    }
  }
  else // Must be 1bpp
  {
    uint8_t * ptr = (uint8_t*)data;
    uint32_t ww =  (w+7)>>3; // Width of source image line in bytes
    for (int32_t yp = z.dy;  yp < z.dy + z.dh; yp++)
    {
      uint8_t* linePtr = (uint8_t*)lineBuf;
      for (int32_t xp = z.dx; xp < z.dx + z.dw; xp++)
      {
        uint16_t col = (pgm_read_byte(ptr + (xp>>3)) & (0x80 >> (xp & 0x7)) );
        if (col) {*linePtr++ = bitmap_fg>>8; *linePtr++ = (uint8_t) bitmap_fg;}
        else     {*linePtr++ = bitmap_bg>>8; *linePtr++ = (uint8_t) bitmap_bg;}
      }
      ptr += ww;
      pushPixels(lineBuf, z.dw, false);
    }
  }

  inTransaction = lockTransaction;
  end_tft_write();
}


/***************************************************************************************
** Function name:           pushImage
** Description:             plot 8 bit or 4 bit or 1 bit image or sprite using a line buffer
***************************************************************************************/
void TFT_GFX::pushImage(clip_t& clip, int32_t x0, int32_t y0, int32_t w, int32_t h, uint16_t bitmap_fg, uint16_t bitmap_bg, uint8_t *data, bool bpp8,  uint16_t *cmap)
{
  block_t z;
  if (!clip.check_block(z, x0, y0, w, h)) return;

  begin_tft_write();
  inTransaction = true;

  setWindow(z.x, z.y, z.dw, z.dh); // Sets CS low and sent RAMWR

  // Line buffer makes plotting faster
  uint16_t  lineBuf[z.dw];

  if (bpp8)
  {
    uint8_t  blue[] = {0, 11, 21, 31}; // blue 2 to 5 bit colour lookup table

    uint32_t _lastColor = -1; // Set to illegal value

    // Used to store last shifted colour
    uint8_t msbColor = 0;
    uint8_t lsbColor = 0;

    data += z.dx + z.dy * w;
    while (z.dh--) {
      uint32_t len = z.dw;
      uint8_t* ptr = data;
      uint8_t* linePtr = (uint8_t*)lineBuf;

      while(len--) {
        uint32_t color = *ptr++;

        // Shifts are slow so check if colour has changed first
        if (color != _lastColor) {
          //          =====Green=====     ===============Red==============
          msbColor = (color & 0x1C)>>2 | (color & 0xC0)>>3 | (color & 0xE0);
          //          =====Green=====    =======Blue======
          lsbColor = (color & 0x1C)<<3 | blue[color & 0x03];
          _lastColor = color;
        }

       *linePtr++ = msbColor;
       *linePtr++ = lsbColor;
      }

      pushPixels(lineBuf, z.dw, false);

      data += w;
    }
  }
  else if (cmap != nullptr) // Must be 4bpp
  {
    w = (w+1) & 0xFFFE;   // if this is a sprite, w will already be even; this does no harm.
    bool splitFirst = (z.dx & 0x01) != 0; // split first means we have to push a single px from the left of the sprite / image

    if (splitFirst) {
      data += ((z.dx - 1 + z.dy * w) >> 1);
    }
    else {
      data += ((z.dx + z.dy * w) >> 1);
    }

    while (z.dh--) {
      uint32_t len = z.dw;
      uint8_t * ptr = data;
      uint16_t *linePtr = lineBuf;
      uint8_t colors; // two colors in one byte
      uint16_t index;

      if (splitFirst) {
        colors = *ptr;
        index = (colors & 0x0F);
        *linePtr++ = cmap[index];
        len--;
        ptr++;
      }

      while (len--)
      {
        colors = *ptr;
        index = ((colors & 0xF0) >> 4) & 0x0F;
        *linePtr++ = cmap[index];

        if (len--)
        {
          index = colors & 0x0F;
          *linePtr++ = cmap[index];
        } else {
          break;  // nothing to do here
        }

        ptr++;
      }

      pushPixels(lineBuf, z.dw, true);
      data += (w >> 1);
    }
  }
  else // Must be 1bpp
  {
    uint32_t ww =  (w+7)>>3; // Width of source image line in bytes
    for (int32_t yp = z.dy;  yp < z.dy + z.dh; yp++)
    {
      uint8_t* linePtr = (uint8_t*)lineBuf;
      for (int32_t xp = z.dx; xp < z.dx + z.dw; xp++)
      {
        uint16_t col = (data[(xp>>3)] & (0x80 >> (xp & 0x7)) );
        if (col) {*linePtr++ = bitmap_fg>>8; *linePtr++ = (uint8_t) bitmap_fg;}
        else     {*linePtr++ = bitmap_bg>>8; *linePtr++ = (uint8_t) bitmap_bg;}
      }
      data += ww;
      pushPixels(lineBuf, z.dw, false);
    }
  }

  inTransaction = lockTransaction;
  end_tft_write();
}


/***************************************************************************************
** Function name:           pushImage
** Description:             plot 8 or 4 or 1 bit image or sprite with a transparent colour
***************************************************************************************/
void TFT_GFX::pushImage(clip_t& clip, int32_t x0, int32_t y0, int32_t w, int32_t h, uint16_t bitmap_fg, uint16_t bitmap_bg, uint8_t *data, uint8_t transp, bool bpp8, uint16_t *cmap)
{
  block_t z;
  if (!clip.check_block(z, x0, y0, w, h)) return;

  begin_tft_write();
  inTransaction = true;


  // Line buffer makes plotting faster
  uint16_t  lineBuf[z.dw];

  if (bpp8) { // 8 bits per pixel
    data += z.dx + z.dy * w;

    uint8_t  blue[] = {0, 11, 21, 31}; // blue 2 to 5 bit colour lookup table

    uint32_t _lastColor = -1; // Set to illegal value

    // Used to store last shifted colour
    uint8_t msbColor = 0;
    uint8_t lsbColor = 0;

    while (z.dh--) {
      int32_t len = z.dw;
      uint8_t* ptr = data;
      uint8_t* linePtr = (uint8_t*)lineBuf;

      int32_t px = z.x, sx = z.x;
      bool move = true;
      uint16_t np = 0;

      while (len--) {
        if (transp != *ptr) {
          if (move) { move = false; sx = px; }
          uint8_t color = *ptr;

          // Shifts are slow so check if colour has changed first
          if (color != _lastColor) {
            //          =====Green=====     ===============Red==============
            msbColor = (color & 0x1C)>>2 | (color & 0xC0)>>3 | (color & 0xE0);
            //          =====Green=====    =======Blue======
            lsbColor = (color & 0x1C)<<3 | blue[color & 0x03];
            _lastColor = color;
          }
          *linePtr++ = msbColor;
          *linePtr++ = lsbColor;
          np++;
        }
        else {
          move = true;
          if (np) {
            setWindow(sx, z.y, np, 1);
            pushPixels(lineBuf, np, false);
            linePtr = (uint8_t*)lineBuf;
            np = 0;
          }
        }
        px++;
        ptr++;
      }

      if (np) { setWindow(sx, z.y, np, 1); pushPixels(lineBuf, np, false); }
      z.y++;
      data += w;
    }
  }
  else if (cmap != nullptr) // 4bpp with color map
  {
    w = (w+1) & 0xFFFE; // here we try to recreate iwidth from dwidth.
    bool splitFirst = ((z.dx & 0x01) != 0);
    if (splitFirst) {
      data += ((z.dx - 1 + z.dy * w) >> 1);
    }
    else {
      data += ((z.dx + z.dy * w) >> 1);
    }

    while (z.dh--) {
      uint32_t len = z.dw;
      uint8_t * ptr = data;

      int32_t px = z.x, sx = z.x;
      bool move = true;
      uint16_t np = 0;

      uint8_t index;  // index into cmap.

      if (splitFirst) {
        index = (*ptr & 0x0F);  // odd = bits 3 .. 0
        if (index != transp) {
          move = false; sx = px;
          lineBuf[np] = cmap[index];
          np++;
        }
        px++; ptr++;
        len--;
      }

      while (len--)
      {
        uint8_t color = *ptr;

        // find the actual color you care about.  There will be two pixels here!
        // but we may only want one at the end of the row
        uint16_t index = ((color & 0xF0) >> 4) & 0x0F;  // high bits are the even numbers
        if (index != transp) {
          if (move) {
            move = false; sx = px;
          }
          lineBuf[np] = cmap[index];
          np++; // added a pixel
        }
        else {
          move = true;
          if (np) {
            setWindow(sx, z.y, np, 1);
            pushPixels(lineBuf, np, true);
            np = 0;
          }
        }
        px++;

        if (len--)
        {
          index = color & 0x0F; // the odd number is 3 .. 0
          if (index != transp) {
            if (move) {
              move = false; sx = px;
             }
            lineBuf[np] = cmap[index];
            np++;
          }
          else {
            move = true;
            if (np) {
              setWindow(sx, z.y, np, 1);
              pushPixels(lineBuf, np, true);
              np = 0;
            }
          }
          px++;
        }
        else {
          break;  // we are done with this row.
        }
        ptr++;  // we only increment ptr once in the loop (deliberate)
      }

      if (np) {
        setWindow(sx, z.y, np, 1);
        pushPixels(lineBuf, np, true);
        np = 0;
      }
      data += (w>>1);
      z.y++;
    }
  }
  else { // 1 bit per pixel
    uint32_t ww =  (w+7)>>3; // Width of source image line in bytes
    uint16_t np = 0;

    for (int32_t yp = z.dy;  yp < z.dy + z.dh; yp++)
    {
      int32_t px = z.x, sx = z.x;
      bool move = true;
      for (int32_t xp = z.dx; xp < z.dx + z.dw; xp++)
      {
        if (data[(xp>>3)] & (0x80 >> (xp & 0x7))) {
          if (move) {
            move = false;
            sx = px;
          }
          np++;
        }
        else {
          move = true;
          if (np) {
            setWindow(sx, z.y, np, 1);
            pushBlock(bitmap_fg, np);
            np = 0;
          }
        }
        px++;
      }
      if (np) { setWindow(sx, z.y, np, 1); pushBlock(bitmap_fg, np); np = 0; }
      z.y++;
      data += ww;
    }
  }
  inTransaction = lockTransaction;
  end_tft_write();
}

/***************************************************************************************
** Function name:           pushMaskedImage
** Description:             Render a 16 bit colour image with a 1bpp mask
***************************************************************************************/
// Can be used with a 16bpp sprite and a 1bpp sprite for the mask
void TFT_GFX::pushMaskedImage(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, bool swapBytes, uint16_t *img, uint8_t *mask)
{
//  if (clip._vpOoB) return;
  if (w < 1 || h < 1) return;

  // To simplify mask handling the window clipping is done by the pushImage function
  // Each mask image line assumed to be padded to and integer number of bytes & padding bits are 0

  begin_tft_write();
  inTransaction = true;

  uint8_t  *mptr = mask;
  uint8_t  *eptr = mask + ((w + 7) >> 3);
  uint16_t *iptr = img;
  uint32_t setCount = 0;

  // For each line in the image
  while (h--) {
    uint32_t xp = 0;
    uint32_t clearCount = 0;
    uint8_t  mbyte= *mptr++;
    uint32_t bits  = 8;
    // Scan through each byte of the bitmap and determine run lengths
    do {
      setCount = 0;

      //Get run length for clear bits to determine x offset
      while ((mbyte & 0x80) == 0x00) {
        // Check if remaining bits in byte are clear (reduce shifts)
        if (mbyte == 0) {
          clearCount += bits;      // bits not always 8 here
          if (mptr >= eptr) break; // end of line
          mbyte = *mptr++;
          bits  = 8;
          continue;
        }
        mbyte = mbyte << 1; // 0's shifted in
        clearCount ++;
        if (--bits) continue;;
        if (mptr >= eptr) break;
        mbyte = *mptr++;
        bits  = 8;
      }

      //Get run length for set bits to determine render width
      while ((mbyte & 0x80) == 0x80) {
        // Check if all bits are set (reduces shifts)
        if (mbyte == 0xFF) {
          setCount += bits;
          if (mptr >= eptr) break;
          mbyte = *mptr++;
          //bits  = 8; // NR, bits always 8 here unless 1's shifted in
          continue;
        }
        mbyte = mbyte << 1; //or mbyte += mbyte + 1 to shift in 1's
        setCount ++;
        if (--bits) continue;
        if (mptr >= eptr) break;
        mbyte = *mptr++;
        bits  = 8;
      }

      // A mask boundary or mask end has been found, so render the pixel line
      if (setCount) {
        xp += clearCount;
        clearCount = 0;
        pushImage(clip, x + xp, y, setCount, 1, swapBytes, iptr + xp);      // pushImage handles clipping
        //pushImageDMA(x + xp, y, setCount, 1, iptr + xp);
        xp += setCount;
      }
    } while (setCount || mptr < eptr);

    y++;
    iptr += w;
    eptr += ((w + 7) >> 3);
  }

  inTransaction = lockTransaction;
  end_tft_write();
}


/***************************************************************************************
** Function name:           drawCircle
** Description:             Draw a circle outline
***************************************************************************************/
// Optimised midpoint circle algorithm
void TFT_GFX::drawCircle(clip_t& clip, int32_t x0, int32_t y0, int32_t r, uint32_t color)
{
  if ( r <= 0 ) return;

  //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
  inTransaction = true;

    int32_t f     = 1 - r;
    int32_t ddF_y = -2 * r;
    int32_t ddF_x = 1;
    int32_t xs    = -1;
    int32_t xe    = 0;
    int32_t len   = 0;

    bool first = true;
    do {
      while (f < 0) {
        ++xe;
        f += (ddF_x += 2);
      }
      f += (ddF_y += 2);

      if (xe-xs>1) {
        if (first) {
          len = 2*(xe - xs)-1;
          drawFastHLine(clip, x0 - xe, y0 + r, len, color);
          drawFastHLine(clip, x0 - xe, y0 - r, len, color);
          drawFastVLine(clip, x0 + r, y0 - xe, len, color);
          drawFastVLine(clip, x0 - r, y0 - xe, len, color);
          first = false;
        }
        else {
          len = xe - xs++;
          drawFastHLine(clip, x0 - xe, y0 + r, len, color);
          drawFastHLine(clip, x0 - xe, y0 - r, len, color);
          drawFastHLine(clip, x0 + xs, y0 - r, len, color);
          drawFastHLine(clip, x0 + xs, y0 + r, len, color);

          drawFastVLine(clip, x0 + r, y0 + xs, len, color);
          drawFastVLine(clip, x0 + r, y0 - xe, len, color);
          drawFastVLine(clip, x0 - r, y0 - xe, len, color);
          drawFastVLine(clip, x0 - r, y0 + xs, len, color);
        }
      }
      else {
        ++xs;
        drawPixel(clip, x0 - xe, y0 + r, color);
        drawPixel(clip, x0 - xe, y0 - r, color);
        drawPixel(clip, x0 + xs, y0 - r, color);
        drawPixel(clip, x0 + xs, y0 + r, color);

        drawPixel(clip, x0 + r, y0 + xs, color);
        drawPixel(clip, x0 + r, y0 - xe, color);
        drawPixel(clip, x0 - r, y0 - xe, color);
        drawPixel(clip, x0 - r, y0 + xs, color);
      }
      xs = xe;
    } while (xe < --r);

  inTransaction = lockTransaction;
  end_tft_write();              // Does nothing if Sprite class uses this function
}


/***************************************************************************************
** Function name:           drawCircleHelper
** Description:             Support function for drawRoundRect()
***************************************************************************************/
void TFT_GFX::drawCircleHelper(clip_t& clip, int32_t x0, int32_t y0, int32_t rr, uint8_t cornername, uint32_t color)
{
  if (rr <= 0) return;
  int32_t f     = 1 - rr;
  int32_t ddF_x = 1;
  int32_t ddF_y = -2 * rr;
  int32_t xe    = 0;
  int32_t xs    = 0;
  int32_t len   = 0;

  //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
  inTransaction = true;

  while (xe < rr--)
  {
    while (f < 0) {
      ++xe;
      f += (ddF_x += 2);
    }
    f += (ddF_y += 2);

    if (xe-xs==1) {
      if (cornername & 0x1) { // left top
        drawPixel(clip, x0 - xe, y0 - rr, color);
        drawPixel(clip, x0 - rr, y0 - xe, color);
      }
      if (cornername & 0x2) { // right top
        drawPixel(clip, x0 + rr    , y0 - xe, color);
        drawPixel(clip, x0 + xs + 1, y0 - rr, color);
      }
      if (cornername & 0x4) { // right bottom
        drawPixel(clip, x0 + xs + 1, y0 + rr    , color);
        drawPixel(clip, x0 + rr, y0 + xs + 1, color);
      }
      if (cornername & 0x8) { // left bottom
        drawPixel(clip, x0 - rr, y0 + xs + 1, color);
        drawPixel(clip, x0 - xe, y0 + rr    , color);
      }
    }
    else {
      len = xe - xs++;
      if (cornername & 0x1) { // left top
        drawFastHLine(clip, x0 - xe, y0 - rr, len, color);
        drawFastVLine(clip, x0 - rr, y0 - xe, len, color);
      }
      if (cornername & 0x2) { // right top
        drawFastVLine(clip, x0 + rr, y0 - xe, len, color);
        drawFastHLine(clip, x0 + xs, y0 - rr, len, color);
      }
      if (cornername & 0x4) { // right bottom
        drawFastHLine(clip, x0 + xs, y0 + rr, len, color);
        drawFastVLine(clip, x0 + rr, y0 + xs, len, color);
      }
      if (cornername & 0x8) { // left bottom
        drawFastVLine(clip, x0 - rr, y0 + xs, len, color);
        drawFastHLine(clip, x0 - xe, y0 + rr, len, color);
      }
    }
    xs = xe;
  }
  inTransaction = lockTransaction;
  end_tft_write();              // Does nothing if Sprite class uses this function
}

/***************************************************************************************
** Function name:           fillCircle
** Description:             draw a filled circle
***************************************************************************************/
// Optimised midpoint circle algorithm, changed to horizontal lines (faster in sprites)
// Improved algorithm avoids repetition of lines
void TFT_GFX::fillCircle(clip_t& clip, int32_t x0, int32_t y0, int32_t r, uint32_t color)
{
  int32_t  x  = 0;
  int32_t  dx = 1;
  int32_t  dy = r+r;
  int32_t  p  = -(r>>1);

  //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
  inTransaction = true;

  drawFastHLine(clip, x0 - r, y0, dy+1, color);

  while(x<r){

    if(p>=0) {
      drawFastHLine(clip, x0 - x, y0 + r, dx, color);
      drawFastHLine(clip, x0 - x, y0 - r, dx, color);
      dy-=2;
      p-=dy;
      r--;
    }

    dx+=2;
    p+=dx;
    x++;

    drawFastHLine(clip, x0 - r, y0 + x, dy+1, color);
    drawFastHLine(clip, x0 - r, y0 - x, dy+1, color);

  }

  inTransaction = lockTransaction;
  end_tft_write();              // Does nothing if Sprite class uses this function
}

/***************************************************************************************
** Function name:           fillCircleHelper
** Description:             Support function for fillRoundRect()
***************************************************************************************/
// Support drawing roundrects, changed to horizontal lines (faster in sprites)
void TFT_GFX::fillCircleHelper(clip_t& clip, int32_t x0, int32_t y0, int32_t r, uint8_t cornername, int32_t delta, uint32_t color)
{
  int32_t f     = 1 - r;
  int32_t ddF_x = 1;
  int32_t ddF_y = -r - r;
  int32_t y     = 0;

  delta++;

  while (y < r) {
    if (f >= 0) {
      if (cornername & 0x1) drawFastHLine(clip, x0 - y, y0 + r, y + y + delta, color);
      if (cornername & 0x2) drawFastHLine(clip, x0 - y, y0 - r, y + y + delta, color);
      r--;
      ddF_y += 2;
      f     += ddF_y;
    }

    y++;
    ddF_x += 2;
    f     += ddF_x;

    if (cornername & 0x1) drawFastHLine(clip, x0 - r, y0 + y, r + r + delta, color);
    if (cornername & 0x2) drawFastHLine(clip, x0 - r, y0 - y, r + r + delta, color);
  }
}


/***************************************************************************************
** Function name:           drawEllipse
** Description:             Draw a ellipse outline
***************************************************************************************/
void TFT_GFX::drawEllipse(clip_t& clip, int16_t x0, int16_t y0, int32_t rx, int32_t ry, uint16_t color)
{
  if (rx<2) return;
  if (ry<2) return;
  int32_t x, y;
  int32_t rx2 = rx * rx;
  int32_t ry2 = ry * ry;
  int32_t fx2 = 4 * rx2;
  int32_t fy2 = 4 * ry2;
  int32_t s;

  //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
  inTransaction = true;

  for (x = 0, y = ry, s = 2*ry2+rx2*(1-2*ry); ry2*x <= rx2*y; x++) {
    // These are ordered to minimise coordinate changes in x or y
    // drawPixel can then send fewer bounding box commands
    drawPixel(clip, x0 + x, y0 + y, color);
    drawPixel(clip, x0 - x, y0 + y, color);
    drawPixel(clip, x0 - x, y0 - y, color);
    drawPixel(clip, x0 + x, y0 - y, color);
    if (s >= 0) {
      s += fx2 * (1 - y);
      y--;
    }
    s += ry2 * ((4 * x) + 6);
  }

  for (x = rx, y = 0, s = 2*rx2+ry2*(1-2*rx); rx2*y <= ry2*x; y++) {
    // These are ordered to minimise coordinate changes in x or y
    // drawPixel can then send fewer bounding box commands
    drawPixel(clip, x0 + x, y0 + y, color);
    drawPixel(clip, x0 - x, y0 + y, color);
    drawPixel(clip, x0 - x, y0 - y, color);
    drawPixel(clip, x0 + x, y0 - y, color);
    if (s >= 0)
    {
      s += fy2 * (1 - x);
      x--;
    }
    s += rx2 * ((4 * y) + 6);
  }

  inTransaction = lockTransaction;
  end_tft_write();              // Does nothing if Sprite class uses this function
}


/***************************************************************************************
** Function name:           fillEllipse
** Description:             draw a filled ellipse
***************************************************************************************/
void TFT_GFX::fillEllipse(clip_t& clip, int16_t x0, int16_t y0, int32_t rx, int32_t ry, uint16_t color)
{
  if (rx<2) return;
  if (ry<2) return;
  int32_t x, y;
  int32_t rx2 = rx * rx;
  int32_t ry2 = ry * ry;
  int32_t fx2 = 4 * rx2;
  int32_t fy2 = 4 * ry2;
  int32_t s;

  //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
  inTransaction = true;

  for (x = 0, y = ry, s = 2*ry2+rx2*(1-2*ry); ry2*x <= rx2*y; x++) {
    drawFastHLine(clip, x0 - x, y0 - y, x + x + 1, color);
    drawFastHLine(clip, x0 - x, y0 + y, x + x + 1, color);

    if (s >= 0) {
      s += fx2 * (1 - y);
      y--;
    }
    s += ry2 * ((4 * x) + 6);
  }

  for (x = rx, y = 0, s = 2*rx2+ry2*(1-2*rx); rx2*y <= ry2*x; y++) {
    drawFastHLine(clip, x0 - x, y0 - y, x + x + 1, color);
    drawFastHLine(clip, x0 - x, y0 + y, x + x + 1, color);

    if (s >= 0) {
      s += fy2 * (1 - x);
      x--;
    }
    s += rx2 * ((4 * y) + 6);
  }

  inTransaction = lockTransaction;
  end_tft_write();              // Does nothing if Sprite class uses this function
}


/***************************************************************************************
** Function name:           drawRect
** Description:             Draw a rectangle outline
***************************************************************************************/
// Draw a rectangle
void TFT_GFX::drawRect(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color)
{
  //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
  inTransaction = true;

  drawFastHLine(clip, x, y, w, color);
  drawFastHLine(clip, x, y + h - 1, w, color);
  // Avoid drawing corner pixels twice
  drawFastVLine(clip, x, y+1, h-2, color);
  drawFastVLine(clip, x + w - 1, y+1, h-2, color);

  inTransaction = lockTransaction;
  end_tft_write();              // Does nothing if Sprite class uses this function
}


/***************************************************************************************
** Function name:           drawRoundRect
** Description:             Draw a rounded corner rectangle outline
***************************************************************************************/
// Draw a rounded rectangle
void TFT_GFX::drawRoundRect(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color)
{
  //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
  inTransaction = true;

  // smarter version
  drawFastHLine(clip, x + r  , y    , w - r - r, color); // Top
  drawFastHLine(clip, x + r  , y + h - 1, w - r - r, color); // Bottom
  drawFastVLine(clip, x    , y + r  , h - r - r, color); // Left
  drawFastVLine(clip, x + w - 1, y + r  , h - r - r, color); // Right
  // draw four corners
  drawCircleHelper(clip, x + r    , y + r    , r, 1, color);
  drawCircleHelper(clip, x + w - r - 1, y + r    , r, 2, color);
  drawCircleHelper(clip, x + w - r - 1, y + h - r - 1, r, 4, color);
  drawCircleHelper(clip, x + r    , y + h - r - 1, r, 8, color);

  inTransaction = lockTransaction;
  end_tft_write();              // Does nothing if Sprite class uses this function
}


/***************************************************************************************
** Function name:           fillRoundRect
** Description:             Draw a rounded corner filled rectangle
***************************************************************************************/
// Fill a rounded rectangle, changed to horizontal lines (faster in sprites)
void TFT_GFX::fillRoundRect(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color)
{
  //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
  inTransaction = true;

  // smarter version
  fillRect(clip, x, y + r, w, h - r - r, color);

  // draw four corners
  fillCircleHelper(clip, x + r, y + h - r - 1, r, 1, w - r - r - 1, color);
  fillCircleHelper(clip, x + r    , y + r, r, 2, w - r - r - 1, color);

  inTransaction = lockTransaction;
  end_tft_write();              // Does nothing if Sprite class uses this function
}


/***************************************************************************************
** Function name:           drawTriangle
** Description:             Draw a triangle outline using 3 arbitrary points
***************************************************************************************/
// Draw a triangle
void TFT_GFX::drawTriangle(clip_t& clip, int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
{
  //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
  inTransaction = true;

  drawLine(clip, x0, y0, x1, y1, color);
  drawLine(clip, x1, y1, x2, y2, color);
  drawLine(clip, x2, y2, x0, y0, color);

  inTransaction = lockTransaction;
  end_tft_write();              // Does nothing if Sprite class uses this function
}


/***************************************************************************************
** Function name:           fillTriangle
** Description:             Draw a filled triangle using 3 arbitrary points
***************************************************************************************/
// Fill a triangle - original Adafruit function works well and code footprint is small
void TFT_GFX::fillTriangle (clip_t&clip, int32_t x0, int32_t y0, int32_t x1, int32_t y1, int32_t x2, int32_t y2, uint32_t color)
{
  int32_t a, b, y, last;

  // Sort coordinates by Y order (y2 >= y1 >= y0)
  if (y0 > y1) {
    transpose(y0, y1); transpose(x0, x1);
  }
  if (y1 > y2) {
    transpose(y2, y1); transpose(x2, x1);
  }
  if (y0 > y1) {
    transpose(y0, y1); transpose(x0, x1);
  }

  if (y0 == y2) { // Handle awkward all-on-same-line case as its own thing
    a = b = x0;
    if (x1 < a)      a = x1;
    else if (x1 > b) b = x1;
    if (x2 < a)      a = x2;
    else if (x2 > b) b = x2;
    drawFastHLine(clip, a, y0, b - a + 1, color);
    return;
  }

  //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
  inTransaction = true;

  int32_t
  dx01 = x1 - x0,
  dy01 = y1 - y0,
  dx02 = x2 - x0,
  dy02 = y2 - y0,
  dx12 = x2 - x1,
  dy12 = y2 - y1,
  sa   = 0,
  sb   = 0;

  // For upper part of triangle, find scanline crossings for segments
  // 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
  // is included here (and second loop will be skipped, avoiding a /0
  // error there), otherwise scanline y1 is skipped here and handled
  // in the second loop...which also avoids a /0 error here if y0=y1
  // (flat-topped triangle).
  if (y1 == y2) last = y1;  // Include y1 scanline
  else         last = y1 - 1; // Skip it

  for (y = y0; y <= last; y++) {
    a   = x0 + sa / dy01;
    b   = x0 + sb / dy02;
    sa += dx01;
    sb += dx02;

    if (a > b) transpose(a, b);
    drawFastHLine(clip, a, y, b - a + 1, color);
  }

  // For lower part of triangle, find scanline crossings for segments
  // 0-2 and 1-2.  This loop is skipped if y1=y2.
  sa = dx12 * (y - y1);
  sb = dx02 * (y - y0);
  for (; y <= y2; y++) {
    a   = x1 + sa / dy12;
    b   = x0 + sb / dy02;
    sa += dx12;
    sb += dx02;

    if (a > b) transpose(a, b);
    drawFastHLine(clip, a, y, b - a + 1, color);
  }

  inTransaction = lockTransaction;
  end_tft_write();              // Does nothing if Sprite class uses this function
}


/***************************************************************************************
** Function name:           drawBitmap
** Description:             Draw an image stored in an array on the TFT
***************************************************************************************/
void TFT_GFX::drawBitmap(clip_t& clip, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{
  //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
  inTransaction = true;

  int32_t i, j, byteWidth = (w + 7) / 8;

  for (j = 0; j < h; j++) {
    for (i = 0; i < w; i++ ) {
      if (pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7))) {
        drawPixel(clip, x + i, y + j, color);
      }
    }
  }

  inTransaction = lockTransaction;
  end_tft_write();              // Does nothing if Sprite class uses this function
}


/***************************************************************************************
** Function name:           drawBitmap
** Description:             Draw an image stored in an array on the TFT
***************************************************************************************/
void TFT_GFX::drawBitmap(clip_t& clip, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t fgcolor, uint16_t bgcolor)
{
  //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
  inTransaction = true;

  int32_t i, j, byteWidth = (w + 7) / 8;

  for (j = 0; j < h; j++) {
    for (i = 0; i < w; i++ ) {
      if (pgm_read_byte(bitmap + j * byteWidth + i / 8) & (128 >> (i & 7)))
           drawPixel(clip, x + i, y + j, fgcolor);
      else drawPixel(clip, x + i, y + j, bgcolor);
    }
  }

  inTransaction = lockTransaction;
  end_tft_write();              // Does nothing if Sprite class uses this function
}

/***************************************************************************************
** Function name:           drawXBitmap
** Description:             Draw an image stored in an XBM array onto the TFT
***************************************************************************************/
void TFT_GFX::drawXBitmap(clip_t& clip, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color)
{
  //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
  inTransaction = true;

  int32_t i, j, byteWidth = (w + 7) / 8;

  for (j = 0; j < h; j++) {
    for (i = 0; i < w; i++ ) {
      if (pgm_read_byte(bitmap + j * byteWidth + i / 8) & (1 << (i & 7))) {
        drawPixel(clip, x + i, y + j, color);
      }
    }
  }

  inTransaction = lockTransaction;
  end_tft_write();              // Does nothing if Sprite class uses this function
}


/***************************************************************************************
** Function name:           drawXBitmap
** Description:             Draw an XBM image with foreground and background colors
***************************************************************************************/
void TFT_GFX::drawXBitmap(clip_t& clip, int16_t x, int16_t y, const uint8_t *bitmap, int16_t w, int16_t h, uint16_t color, uint16_t bgcolor)
{
  //begin_tft_write();          // Sprite class can use this function, avoiding begin_tft_write()
  inTransaction = true;

  int32_t i, j, byteWidth = (w + 7) / 8;

  for (j = 0; j < h; j++) {
    for (i = 0; i < w; i++ ) {
      if (pgm_read_byte(bitmap + j * byteWidth + i / 8) & (1 << (i & 7)))
           drawPixel(clip, x + i, y + j,   color);
      else drawPixel(clip, x + i, y + j, bgcolor);
    }
  }

  inTransaction = lockTransaction;
  end_tft_write();              // Does nothing if Sprite class uses this function
}


/***************************************************************************************
** Function name:           drawLine
** Description:             draw a line between 2 arbitrary points
***************************************************************************************/
// Bresenham's algorithm - thx wikipedia - speed enhanced by Bodmer to use
// an efficient FastH/V Line draw routine for line segments of 2 pixels or more
void TFT_GFX::drawLine(clip_t& clip, int32_t x0, int32_t y0, int32_t x1, int32_t y1, uint32_t color)
{
//  if (clip._vpOoB) return;

  //begin_tft_write();       // Sprite class can use this function, avoiding begin_tft_write()
  inTransaction = true;

  //x+= clip._xDatum;             // Not added here, added by drawPixel & drawFastXLine
  //y+= clip._yDatum;

  bool steep = abs(y1 - y0) > abs(x1 - x0);
  if (steep) {
    transpose(x0, y0);
    transpose(x1, y1);
  }

  if (x0 > x1) {
    transpose(x0, x1);
    transpose(y0, y1);
  }

  int32_t dx = x1 - x0, dy = abs(y1 - y0);;

  int32_t err = dx >> 1, ystep = -1, xs = x0, dlen = 0;

  if (y0 < y1) ystep = 1;

  // Split into steep and not steep for FastH/V separation
  if (steep) {
    for (; x0 <= x1; x0++) {
      dlen++;
      err -= dy;
      if (err < 0) {
        if (dlen == 1) drawPixel(clip, y0, xs, color);
        else drawFastVLine(clip, y0, xs, dlen, color);
        dlen = 0;
        y0 += ystep; xs = x0 + 1;
        err += dx;
      }
    }
    if (dlen) drawFastVLine(clip, y0, xs, dlen, color);
  }
  else
  {
    for (; x0 <= x1; x0++) {
      dlen++;
      err -= dy;
      if (err < 0) {
        if (dlen == 1) drawPixel(clip, xs, y0, color);
        else drawFastHLine(clip, xs, y0, dlen, color);
        dlen = 0;
        y0 += ystep; xs = x0 + 1;
        err += dx;
      }
    }
    if (dlen) drawFastHLine(clip, xs, y0, dlen, color);
  }

  inTransaction = lockTransaction;
  end_tft_write();
}


/***************************************************************************************
** Description:  Constants for anti-aliased line drawing on TFT and in Sprites
***************************************************************************************/
constexpr float PixelAlphaGain   = 255.0;
constexpr float LoAlphaTheshold  = 1.0/32.0;
constexpr float HiAlphaTheshold  = 1.0 - LoAlphaTheshold;
constexpr float deg2rad      = 3.14159265359/180.0;


/***************************************************************************************
** Function name:           drawPixel (alpha blended)
** Description:             Draw a pixel blended with the screen or bg pixel colour
***************************************************************************************/
uint16_t TFT_GFX::drawPixel(clip_t& clip, int32_t x, int32_t y, uint32_t color, uint8_t alpha, uint32_t bg_color)
{
  if (bg_color == 0x00FFFFFF) bg_color = readPixel(clip, x, y);
  color = alphaBlend(alpha, color, bg_color);
  drawPixel(clip, x, y, color);
  return color;
}


/***************************************************************************************
** Function name:           drawSmoothArc
** Description:             Draw a smooth arc clockwise from 6 o'clock
***************************************************************************************/
void TFT_GFX::drawSmoothArc(wh_clip_t& clip, int32_t x, int32_t y, int32_t r, int32_t ir, int32_t startAngle, int32_t endAngle, uint32_t fg_color, uint32_t bg_color, bool roundEnds)
// Centre at x,y
// r = arc outer radius, ir = arc inner radius. Inclusive so arc thickness = r - ir + 1
// Angles in range 0-360
// Arc foreground colour anti-aliased with background colour at edges
// anti-aliased roundEnd is optional, default is anti-aliased straight end
// Note: rounded ends extend the arc angle so can overlap, user sketch to manage this.
{
  inTransaction = true;

  if (endAngle != startAngle && (startAngle != 0 || endAngle != 360))
  {
    float sx = -sinf(startAngle * deg2rad);
    float sy = +cosf(startAngle * deg2rad);
    float ex = -sinf(  endAngle * deg2rad);
    float ey = +cosf(  endAngle * deg2rad);

    if (roundEnds)
    { // Round ends
      sx = sx * (r + ir)/2.0 + x;
      sy = sy * (r + ir)/2.0 + y;
      drawSpot(clip, sx, sy, (r - ir)/2.0, fg_color, bg_color);

      ex = ex * (r + ir)/2.0 + x;
      ey = ey * (r + ir)/2.0 + y;
      drawSpot(clip, ex, ey, (r - ir)/2.0, fg_color, bg_color);
    }
    else
    { // Square ends
      float asx = sx * ir + x;
      float asy = sy * ir + y;
      float aex = sx *  r + x;
      float aey = sy *  r + y;
      drawWedgeLine(clip, asx, asy, aex, aey, 0.3, 0.3, fg_color, bg_color);

      asx = ex * ir + x;
      asy = ey * ir + y;
      aex = ex *  r + x;
      aey = ey *  r + y;
      drawWedgeLine(clip, asx, asy, aex, aey, 0.3, 0.3, fg_color, bg_color);
    }

    // Draw arc
    drawArc(clip, x, y, r, ir, startAngle, endAngle, fg_color, bg_color);

  }
  else // Draw full 360
  {
    drawArc(clip, x, y, r, ir, 0, 360, fg_color, bg_color);
  }

  inTransaction = lockTransaction;
  end_tft_write();
}


/***************************************************************************************
** Function name:           sqrt_fraction (private function)
** Description:             Smooth graphics support function for alpha derivation
***************************************************************************************/
// Compute the fixed point square root of an integer and
// return the 8 MS bits of fractional part.
// Quicker than sqrt() for processors that do not have and FPU (e.g. RP2040)
inline uint8_t TFT_GFX::sqrt_fraction(uint32_t num) {
  if (num > (0x40000000)) return 0;
  uint32_t bsh = 0x00004000;
  uint32_t fpr = 0;
  uint32_t osh = 0;

  // Auto adjust from U8:8 up to U15:16
  while (num>bsh) {bsh <<= 2; osh++;}

  do {
    uint32_t bod = bsh + fpr;
    if(num >= bod)
    {
      num -= bod;
      fpr = bsh + bod;
    }
    num <<= 1;
  } while(bsh >>= 1);

  return fpr>>osh;
}

/***************************************************************************************
** Function name:           drawArc
** Description:             Draw an arc clockwise from 6 o'clock position
***************************************************************************************/
// Centre at x,y
// r = arc outer radius, ir = arc inner radius. Inclusive, so arc thickness = r-ir+1
// Angles MUST be in range 0-360
// Arc foreground fg_color anti-aliased with background colour along sides
// smooth is optional, default is true, smooth=false means no antialiasing
// Note: Arc ends are not anti-aliased (use drawSmoothArc instead for that)
void TFT_GFX::drawArc(clip_t& clip, int32_t x, int32_t y, int32_t r, int32_t ir,
                       int32_t startAngle, int32_t endAngle,
                       uint32_t fg_color, uint32_t bg_color,
                       bool smooth)
{
  if (endAngle < startAngle) {
    // Arc sweeps through 6 o'clock so draw in two parts
    drawArc(clip, x, y, r, ir, startAngle, 360, fg_color, bg_color, smooth);
    startAngle = 0;
  }

//  if (clip._vpOoB) return;
  if (startAngle == endAngle) return;
  if (r < ir) transpose(r, ir);  // Required that r > ir
  if (r <= 0 || ir < 0) return;  // Invalid r, ir can be zero (circle sector)
  if (startAngle < 0) startAngle = 0;
  if (endAngle > 360) endAngle = 360;

  inTransaction = true;

  int32_t xs = 0;       // x start position for quadrant scan
  uint8_t alpha = 0;    // alpha value for blending pixels

  uint32_t r2 = r * r;   // Outer arc radius^2
  if (smooth) r++;      // Outer AA zone radius
  uint32_t r1 = r * r;   // Outer AA radius^2
  int16_t w  = r - ir;  // Width of arc (r - ir + 1)
  uint32_t r3 = ir * ir; // Inner arc radius^2
  if (smooth) ir--;     // Inner AA zone radius
  uint32_t r4 = ir * ir; // Inner AA radius^2

  // Float variants of adjusted inner and outer arc radii
  //float irf = ir;
  //float rf  = r;

  //     1 | 2
  //    ---¦---    Arc quadrant index
  //     0 | 3
  // Fixed point U16.16 slope table for arc start/end in each quadrant
  uint32_t startSlope[4] = {0, 0, 0xFFFFFFFF, 0};
  uint32_t   endSlope[4] = {0, 0xFFFFFFFF, 0, 0};

  // Ensure maximum U16.16 slope of arc ends is ~ 0x8000 0000
  constexpr float minDivisor = 1.0f/0x8000;

  // Fill in start slope table and empty quadrants
  float fabscos = fabsf(cosf(startAngle * deg2rad));
  float fabssin = fabsf(sinf(startAngle * deg2rad));

  // U16.16 slope of arc start
  uint32_t slope = (fabscos/(fabssin + minDivisor)) * (float)(1<<16);

  // Update slope table, add slope for arc start
  if (startAngle <= 90) {
    startSlope[0] =  slope;
  }
  else if (startAngle <= 180) {
    startSlope[1] =  slope;
  }
  else if (startAngle <= 270) {
    startSlope[1] = 0xFFFFFFFF;
    startSlope[2] = slope;
  }
  else {
    startSlope[1] = 0xFFFFFFFF;
    startSlope[2] =  0;
    startSlope[3] = slope;
  }

  // Fill in end slope table and empty quadrants
  fabscos  = fabsf(cosf(endAngle * deg2rad));
  fabssin  = fabsf(sinf(endAngle * deg2rad));

  // U16.16 slope of arc end
  slope   = (uint32_t)((fabscos/(fabssin + minDivisor)) * (float)(1<<16));

  // Work out which quadrants will need to be drawn and add slope for arc end
  if (endAngle <= 90) {
    endSlope[0] = slope;
    endSlope[1] =  0;
    startSlope[2] =  0;
  }
  else if (endAngle <= 180) {
    endSlope[1] = slope;
    startSlope[2] =  0;
  }
  else if (endAngle <= 270) {
    endSlope[2] =  slope;
  }
  else {
    endSlope[3] =  slope;
  }

  // Scan quadrant
  for (int32_t cy = r - 1; cy > 0; cy--)
  {
    uint32_t len[4] = { 0,  0,  0,  0}; // Pixel run length
    int32_t  xst[4] = {-1, -1, -1, -1}; // Pixel run x start
    uint32_t dy2 = (r - cy) * (r - cy);

    // Find and track arc zone start point
    while ((r - xs) * (r - xs) + dy2 >= r1) xs++;

    for (int32_t cx = xs; cx < r; cx++)
    {
      // Calculate radius^2
      uint32_t hyp = (r - cx) * (r - cx) + dy2;

      // If in outer zone calculate alpha
      if (hyp > r2) {
        //alpha = (uint8_t)((rf - sqrtf(hyp)) * 255);
        alpha = ~sqrt_fraction(hyp); // Outer AA zone
      }
      // If within arc fill zone, get line start and lengths for each quadrant
      else if (hyp >= r3) {
        do {
          // Calculate U16.16 slope
          slope = ((r - cy) << 16)/(r - cx);
          if (slope <= startSlope[0] && slope >= endSlope[0]) { // slope hi -> lo
            xst[0] = cx; // Bottom left line end
            len[0]++;
          }
          if (slope >= startSlope[1] && slope <= endSlope[1]) { // slope lo -> hi
            xst[1] = cx; // Top left line end
            len[1]++;
          }
          if (slope <= startSlope[2] && slope >= endSlope[2]) { // slope hi -> lo
            xst[2] = cx; // Bottom right line start
            len[2]++;
          }
          if (slope <= endSlope[3] && slope >= startSlope[3]) { // slope lo -> hi
            xst[3] = cx; // Top right line start
            len[3]++;
          }
          cx++;
        } while ((r - cx) * (r - cx) + dy2 >= r3 && cx < r);
        cx--;
        continue; // Next x
      }
      else {
        if (hyp <= r4) break;  // Skip inner pixels
        //alpha = (uint8_t)((sqrtf(hyp) - irf) * 255.0);
        alpha = sqrt_fraction(hyp); // Inner AA zone
      }

      if (alpha < 16) continue;  // Skip low alpha pixels

      // If background is read it must be done in each quadrant
      uint16_t pcol = alphaBlend(alpha, fg_color, bg_color);
      // Check if an AA pixels need to be drawn
      slope = ((r - cy)<<16)/(r - cx);
      if (slope <= startSlope[0] && slope >= endSlope[0]) // BL
        drawPixel(clip, x + cx - r, y - cy + r, pcol);
      if (slope >= startSlope[1] && slope <= endSlope[1]) // TL
        drawPixel(clip, x + cx - r, y + cy - r, pcol);
      if (slope <= startSlope[2] && slope >= endSlope[2]) // TR
        drawPixel(clip, x - cx + r, y + cy - r, pcol);
      if (slope <= endSlope[3] && slope >= startSlope[3]) // BR
        drawPixel(clip, x - cx + r, y - cy + r, pcol);
    }
    // Add line in inner zone
    if (len[0]) drawFastHLine(clip, x + xst[0] - len[0] + 1 - r, y - cy + r, len[0], fg_color); // BL
    if (len[1]) drawFastHLine(clip, x + xst[1] - len[1] + 1 - r, y + cy - r, len[1], fg_color); // TL
    if (len[2]) drawFastHLine(clip, x - xst[2] + r, y + cy - r, len[2], fg_color); // TR
    if (len[3]) drawFastHLine(clip, x - xst[3] + r, y - cy + r, len[3], fg_color); // BR
  }

  // Fill in centre lines
  if (startAngle ==   0 || endAngle == 360) drawFastVLine(clip, x, y + r - w, w, fg_color); // Bottom
  if (startAngle <=  90 && endAngle >=  90) drawFastHLine(clip, x - r + 1, y, w, fg_color); // Left
  if (startAngle <= 180 && endAngle >= 180) drawFastVLine(clip, x, y - r + 1, w, fg_color); // Top
  if (startAngle <= 270 && endAngle >= 270) drawFastHLine(clip, x + r - w, y, w, fg_color); // Right

  inTransaction = lockTransaction;
  end_tft_write();
}

/***************************************************************************************
** Function name:           drawSmoothCircle
** Description:             Draw a smooth circle
***************************************************************************************/
// To have effective anti-aliasing the circle will be 3 pixels thick
void TFT_GFX::drawSmoothCircle(clip_t& clip, int32_t x, int32_t y, int32_t r, uint32_t fg_color, uint32_t bg_color)
{
  drawSmoothRoundRect(clip, x-r, y-r, r, r-1, 0, 0, fg_color, bg_color);
}

/***************************************************************************************
** Function name:           fillSmoothCircle
** Description:             Draw a filled anti-aliased circle
***************************************************************************************/
void TFT_GFX::fillSmoothCircle(clip_t& clip, int32_t x, int32_t y, int32_t r, uint32_t color, uint32_t bg_color)
{
  if (r <= 0) return;

  inTransaction = true;

  drawFastHLine(clip, x - r, y, 2 * r + 1, color);
  int32_t xs = 1;
  int32_t cx = 0;

  int32_t r1 = r * r;
  r++;
  int32_t r2 = r * r;

  for (int32_t cy = r - 1; cy > 0; cy--)
  {
    int32_t dy2 = (r - cy) * (r - cy);
    for (cx = xs; cx < r; cx++)
    {
      int32_t hyp2 = (r - cx) * (r - cx) + dy2;
      if (hyp2 <= r1) break;
      if (hyp2 >= r2) continue;
//*
      uint8_t alpha = ~sqrt_fraction(hyp2);
      if (alpha > 246) break;
      xs = cx;
      if (alpha < 9) continue;
      //*/
/*
      float alphaf = (float)r - sqrtf(hyp2);
      if (alphaf > HiAlphaTheshold) break;
      xs = cx;
      if (alphaf < LoAlphaTheshold) continue;
      uint8_t alpha = alphaf * 255;
//*/
      if (bg_color == 0x00FFFFFF) {
        drawPixel(clip, x + cx - r, y + cy - r, color, alpha, bg_color);
        drawPixel(clip, x - cx + r, y + cy - r, color, alpha, bg_color);
        drawPixel(clip, x - cx + r, y - cy + r, color, alpha, bg_color);
        drawPixel(clip, x + cx - r, y - cy + r, color, alpha, bg_color);
      }
      else {
        uint16_t pcol = drawPixel(clip, x + cx - r, y + cy - r, color, alpha, bg_color);
        drawPixel(clip, x - cx + r, y + cy - r, pcol);
        drawPixel(clip, x - cx + r, y - cy + r, pcol);
        drawPixel(clip, x + cx - r, y - cy + r, pcol);
      }
    }
    drawFastHLine(clip, x + cx - r, y + cy - r, 2 * (r - cx) + 1, color);
    drawFastHLine(clip, x + cx - r, y - cy + r, 2 * (r - cx) + 1, color);
  }
  inTransaction = lockTransaction;
  end_tft_write();
}


/***************************************************************************************
** Function name:           drawSmoothRoundRect
** Description:             Draw a rounded rectangle
***************************************************************************************/
// x,y is top left corner of bounding box for a complete rounded rectangle
// r = arc outer corner radius, ir = arc inner radius. Arc thickness = r-ir+1
// w and h are width and height of the bounding rectangle
// If w and h are < radius (e.g. 0,0) a circle will be drawn with centre at x+r,y+r
// Arc foreground fg_color anti-aliased with background colour at edges
// A subset of corners can be drawn by specifying a quadrants mask. A bit set in the
// mask means draw that quadrant (all are drawn if parameter missing):
//   0x1 | 0x2
//    ---¦---    Arc quadrant mask select bits (as in drawCircleHelper fn)
//   0x8 | 0x4
void TFT_GFX::drawSmoothRoundRect(clip_t& clip, int32_t x, int32_t y, int32_t r, int32_t ir, int32_t w, int32_t h, uint32_t fg_color, uint32_t bg_color, uint8_t quadrants)
{
//  if (clip._vpOoB) return;
  if (r < ir) transpose(r, ir); // Required that r > ir
  if (r <= 0 || ir < 0) return;  // Invalid

  w -= 2*r;
  h -= 2*r;

  if (w < 0) w = 0;
  if (h < 0) h = 0;

  inTransaction = true;

  x += r;
  y += r;
/*
  float alphaGain = 1.0;
  if (w != 0 || h != 0) {
    if (r - ir < 2) alphaGain = 1.5; // Boost brightness for thin lines
    if (r - ir < 1) alphaGain = 1.7;
  }
*/
  uint16_t t = r - ir + 1;
  int32_t xs = 0;
  int32_t cx = 0;

  int32_t r2 = r * r;   // Outer arc radius^2
  r++;
  int32_t r1 = r * r;   // Outer AA zone radius^2

  int32_t r3 = ir * ir; // Inner arc radius^2
  ir--;
  int32_t r4 = ir * ir; // Inner AA zone radius^2

  //float irf = ir;
  //float rf  = r;
  uint8_t alpha = 0;

  // Scan top left quadrant x y r ir fg_color  bg_color
  for (int32_t cy = r - 1; cy > 0; cy--)
  {
    int32_t len = 0;  // Pixel run length
    int32_t lxst = 0; // Left side run x start
    int32_t rxst = 0; // Right side run x start
    int32_t dy2 = (r - cy) * (r - cy);

    // Find and track arc zone start point
    while ((r - xs) * (r - xs) + dy2 >= r1) xs++;

    for (cx = xs; cx < r; cx++)
    {
      // Calculate radius^2
      int32_t hyp = (r - cx) * (r - cx) + dy2;

      // If in outer zone calculate alpha
      if (hyp > r2) {
        alpha = ~sqrt_fraction(hyp);
        //alpha = (uint8_t)((rf - sqrtf(hyp)) * 255); // Outer AA zone
      }
      // If within arc fill zone, get line lengths for each quadrant
      else if (hyp >= r3) {
        rxst = cx; // Right side start
        len++;     // Line segment length
        continue;  // Next x
      }
      else {
        if (hyp <= r4) break;  // Skip inner pixels
        //alpha = (uint8_t)((sqrtf(hyp) - irf) * 255); // Inner AA zone
        alpha = sqrt_fraction(hyp);
      }

      if (alpha < 16) continue;  // Skip low alpha pixels

      // If background is read it must be done in each quadrant - TODO
      uint16_t pcol = alphaBlend(alpha, fg_color, bg_color);
      if (quadrants & 0x8) drawPixel(clip, x + cx - r, y - cy + r + h, pcol);     // BL
      if (quadrants & 0x1) drawPixel(clip, x + cx - r, y + cy - r, pcol);         // TL
      if (quadrants & 0x2) drawPixel(clip, x - cx + r + w, y + cy - r, pcol);     // TR
      if (quadrants & 0x4) drawPixel(clip, x - cx + r + w, y - cy + r + h, pcol); // BR
    }
    // Fill arc inner zone in each quadrant
    lxst = rxst - len + 1; // Calculate line segment start for left side
    if (quadrants & 0x8) drawFastHLine(clip, x + lxst - r, y - cy + r + h, len, fg_color);     // BL
    if (quadrants & 0x1) drawFastHLine(clip, x + lxst - r, y + cy - r, len, fg_color);         // TL
    if (quadrants & 0x2) drawFastHLine(clip, x - rxst + r + w, y + cy - r, len, fg_color);     // TR
    if (quadrants & 0x4) drawFastHLine(clip, x - rxst + r + w, y - cy + r + h, len, fg_color); // BR
  }

  // Draw sides
  if ((quadrants & 0xC) == 0xC) fillRect(clip, x, y + r - t + h, w + 1, t, fg_color); // Bottom
  if ((quadrants & 0x9) == 0x9) fillRect(clip, x - r + 1, y, t, h + 1, fg_color);     // Left
  if ((quadrants & 0x3) == 0x3) fillRect(clip, x, y - r + 1, w + 1, t, fg_color);     // Top
  if ((quadrants & 0x6) == 0x6) fillRect(clip, x + r - t + w, y, t, h + 1, fg_color); // Right

  inTransaction = lockTransaction;
  end_tft_write();
}

/***************************************************************************************
** Function name:           fillSmoothRoundRect
** Description:             Draw a filled anti-aliased rounded corner rectangle
***************************************************************************************/
void TFT_GFX::fillSmoothRoundRect(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, int32_t r, uint32_t color, uint32_t bg_color)
{
  inTransaction = true;

  int32_t xs = 0;
  int32_t cx = 0;

  // Limit radius to half width or height
  if (r < 0)   r = 0;
  if (r > w/2) r = w/2;
  if (r > h/2) r = h/2;

  y += r;
  h -= 2*r;
  fillRect(clip, x, y, w, h, color);

  h--;
  x += r;
  w -= 2*r+1;

  int32_t r1 = r * r;
  r++;
  int32_t r2 = r * r;

  for (int32_t cy = r - 1; cy > 0; cy--)
  {
    int32_t dy2 = (r - cy) * (r - cy);
    for (cx = xs; cx < r; cx++)
    {
      int32_t hyp2 = (r - cx) * (r - cx) + dy2;
      if (hyp2 <= r1) break;
      if (hyp2 >= r2) continue;

      uint8_t alpha = ~sqrt_fraction(hyp2);
      if (alpha > 246) break;
      xs = cx;
      if (alpha < 9) continue;
/*
      float alphaf = (float)r - sqrtf(hyp2);
      if (alphaf > HiAlphaTheshold) break;
      xs = cx;
      if (alphaf < LoAlphaTheshold) continue;
      uint8_t alpha = alphaf * 255;
*/
      drawPixel(clip, x + cx - r, y + cy - r, color, alpha, bg_color);
      drawPixel(clip, x - cx + r + w, y + cy - r, color, alpha, bg_color);
      drawPixel(clip, x - cx + r + w, y - cy + r + h, color, alpha, bg_color);
      drawPixel(clip, x + cx - r, y - cy + r + h, color, alpha, bg_color);
    }
    drawFastHLine(clip, x + cx - r, y + cy - r, 2 * (r - cx) + 1 + w, color);
    drawFastHLine(clip, x + cx - r, y - cy + r + h, 2 * (r - cx) + 1 + w, color);
  }
  inTransaction = lockTransaction;
  end_tft_write();
}

/***************************************************************************************
** Function name:           drawWedgeLine - background colour specified or pixel read
** Description:             draw an anti-aliased line with different width radiused ends
***************************************************************************************/
void TFT_GFX::drawWedgeLine(wh_clip_t& clip, float ax, float ay, float bx, float by, float ar, float br, uint32_t fg_color, uint32_t bg_color)
{
  if ( (ar < 0.0) || (br < 0.0) )return;
  if ( (fabsf(ax - bx) < 0.01f) && (fabsf(ay - by) < 0.01f) ) bx += 0.01f;  // Avoid divide by zero

  // Find line bounding box
  int32_t x0 = (int32_t)floorf(fminf(ax-ar, bx-br));
  int32_t x1 = (int32_t) ceilf(fmaxf(ax+ar, bx+br));
  int32_t y0 = (int32_t)floorf(fminf(ay-ar, by-br));
  int32_t y1 = (int32_t) ceilf(fmaxf(ay+ar, by+br));

  if (!clip.clipClip(x0, y0, x1, y1)) return;

  // Establish x start and y start
  int32_t ys = ay;
  if ((ax-ar)>(bx-br)) ys = by;

  float rdt = ar - br; // Radius delta
  float alpha = 1.0f;
  ar += 0.5;

  uint16_t bg = bg_color;
  float xpax, ypay, bax = bx - ax, bay = by - ay;

  begin_nin_write();
  inTransaction = true;

  int32_t xs = x0;
  // Scan bounding box from ys down, calculate pixel intensity from distance to line
  for (int32_t yp = ys; yp <= y1; yp++) {
    bool swin = true;  // Flag to start new window area
    bool endX = false; // Flag to skip pixels
    ypay = yp - ay;
    for (int32_t xp = xs; xp <= x1; xp++) {
      if (endX) if (alpha <= LoAlphaTheshold) break;  // Skip right side
      xpax = xp - ax;
      alpha = ar - wedgeLineDistance(xpax, ypay, bax, bay, rdt);
      if (alpha <= LoAlphaTheshold ) continue;
      // Track edge to minimise calculations
      if (!endX) { endX = true; xs = xp; }
      if (alpha > HiAlphaTheshold) {
        if (swin) { setWindow(xp, yp, clip.width-xp, 1); swin = false; }
        pushColor(fg_color);
        continue;
      }
      //Blend color with background and plot
      if (bg_color == 0x00FFFFFF) {
        bg = readPixel(clip, xp, yp); swin = true;
      }
      if (swin) { setWindow(xp, yp, clip.width-xp, 1); swin = false; }
      pushColor(alphaBlend((uint8_t)(alpha * PixelAlphaGain), fg_color, bg));
    }
  }

  // Reset x start to left side of box
  xs = x0;
  // Scan bounding box from ys-1 up, calculate pixel intensity from distance to line
  for (int32_t yp = ys-1; yp >= y0; yp--) {
    bool swin = true;  // Flag to start new window area
    bool endX = false; // Flag to skip pixels
    ypay = yp - ay;
    for (int32_t xp = xs; xp <= x1; xp++) {
      if (endX) if (alpha <= LoAlphaTheshold) break;  // Skip right side of drawn line
      xpax = xp - ax;
      alpha = ar - wedgeLineDistance(xpax, ypay, bax, bay, rdt);
      if (alpha <= LoAlphaTheshold ) continue;
      // Track line boundary
      if (!endX) { endX = true; xs = xp; }
      if (alpha > HiAlphaTheshold) {
        if (swin) { setWindow(xp, yp, clip.width-xp, 1); swin = false; }
        pushColor(fg_color);
        continue;
      }
      //Blend colour with background and plot
      if (bg_color == 0x00FFFFFF) {
        bg = readPixel(clip, xp, yp); swin = true;
      }
      if (swin) { setWindow(xp, yp, clip.width-xp, 1); swin = false; }
      pushColor(alphaBlend((uint8_t)(alpha * PixelAlphaGain), fg_color, bg));
    }
  }

  inTransaction = lockTransaction;
  end_nin_write();
}


/***************************************************************************************
** Function name:           lineDistance - private helper function for drawWedgeLine
** Description:             returns distance of px,py to closest part of a to b wedge
***************************************************************************************/
inline float TFT_GFX::wedgeLineDistance(float xpax, float ypay, float bax, float bay, float dr)
{
  float h = fmaxf(fminf((xpax * bax + ypay * bay) / (bax * bax + bay * bay), 1.0f), 0.0f);
  float dx = xpax - bax * h, dy = ypay - bay * h;
  return sqrtf(dx * dx + dy * dy) + h * dr;
}


/***************************************************************************************
** Function name:           drawFastVLine
** Description:             draw a vertical line
***************************************************************************************/
void TFT_GFX::drawFastVLine(clip_t& clip, int32_t x, int32_t y, int32_t h, uint32_t color)
{
  int32_t w = 1;
  if (!clip.clipRectNoDatum(x, y, w, h)) return;

  begin_tft_write();

  setWindow(x, y, 1, h);

  pushBlock(color, h);

  end_tft_write();
}


/***************************************************************************************
** Function name:           drawFastHLine
** Description:             draw a horizontal line
***************************************************************************************/
void TFT_GFX::drawFastHLine(clip_t& clip, int32_t x, int32_t y, int32_t w, uint32_t color)
{
  int32_t h = 1;
  if (!clip.clipRectNoDatum(x, y, w, h)) return;

  begin_tft_write();

  setWindow(x, y, w, 1);

  pushBlock(color, w);

  end_tft_write();
}


/***************************************************************************************
** Function name:           fillRect
** Description:             draw a filled rectangle
***************************************************************************************/
void TFT_GFX::fillRect(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color)
{
  if (!clip.clipRectNoDatum(x, y, w, h)) return;

  begin_tft_write();

  setWindow(x, y, w, h);

  pushBlock(color, w * h);

  end_tft_write();
}


/***************************************************************************************
** Function name:           fillRectVGradient
** Description:             draw a filled rectangle with a vertical colour gradient
***************************************************************************************/
void TFT_GFX::fillRectVGradient(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color1, uint32_t color2)
{
  if (!clip.clipRect(x, y, w, h)) return;

  begin_nin_write();

  float delta = -255.0/h;
  float alpha = 255.0;
  uint32_t color = color1;

  while (h--) {
    drawFastHLine(clip, x, y++, w, color);
    alpha += delta;
    color = alphaBlend((uint8_t)alpha, color1, color2);
  }

  end_nin_write();
}


/***************************************************************************************
** Function name:           fillRectHGradient
** Description:             draw a filled rectangle with a horizontal colour gradient
***************************************************************************************/
void TFT_GFX::fillRectHGradient(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color1, uint32_t color2)
{
  if (!clip.clipRect(x, y, w, h)) return;

  begin_nin_write();

  float delta = -255.0/w;
  float alpha = 255.0;
  uint32_t color = color1;

  while (w--) {
    drawFastVLine(clip, x++, y, h, color);
    alpha += delta;
    color = alphaBlend((uint8_t)alpha, color1, color2);
  }

  end_nin_write();
}


/***************************************************************************************
** Function name:           color565
** Description:             convert three 8 bit RGB levels to a 16 bit colour value
***************************************************************************************/
uint16_t TFT_GFX::color565(uint8_t r, uint8_t g, uint8_t b)
{
  return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}


/***************************************************************************************
** Function name:           color16to8
** Description:             convert 16 bit colour to an 8 bit 332 RGB colour value
***************************************************************************************/
uint8_t TFT_GFX::color16to8(uint16_t c)
{
  return ((c & 0xE000)>>8) | ((c & 0x0700)>>6) | ((c & 0x0018)>>3);
}


/***************************************************************************************
** Function name:           color8to16
** Description:             convert 8 bit colour to a 16 bit 565 colour value
***************************************************************************************/
uint16_t TFT_GFX::color8to16(uint8_t color)
{
  uint8_t  blue[] = {0, 11, 21, 31}; // blue 2 to 5 bit colour lookup table
  uint16_t color16 = 0;

  //        =====Green=====     ===============Red==============
  color16  = (color & 0x1C)<<6 | (color & 0xC0)<<5 | (color & 0xE0)<<8;
  //        =====Green=====    =======Blue======
  color16 |= (color & 0x1C)<<3 | blue[color & 0x03];

  return color16;
}

/***************************************************************************************
** Function name:           color16to24
** Description:             convert 16 bit colour to a 24 bit 888 colour value
***************************************************************************************/
uint32_t TFT_GFX::color16to24(uint16_t color565)
{
  uint8_t r = (color565 >> 8) & 0xF8; r |= (r >> 5);
  uint8_t g = (color565 >> 3) & 0xFC; g |= (g >> 6);
  uint8_t b = (color565 << 3) & 0xF8; b |= (b >> 5);

  return ((uint32_t)r << 16) | ((uint32_t)g << 8) | ((uint32_t)b << 0);
}

/***************************************************************************************
** Function name:           color24to16
** Description:             convert 24 bit colour to a 16 bit 565 colour value
***************************************************************************************/
uint32_t TFT_GFX::color24to16(uint32_t color888)
{
  uint16_t r = (color888 >> 8) & 0xF800;
  uint16_t g = (color888 >> 5) & 0x07E0;
  uint16_t b = (color888 >> 3) & 0x001F;

  return (r | g | b);
}


/***************************************************************************************
** Function name:           alphaBlend
** Description:             Blend 16bit foreground and background
*************************************************************************************x*/
uint16_t TFT_GFX::alphaBlend(uint8_t alpha, uint16_t fgc, uint16_t bgc)
{
  // Split out and blend 5 bit red and blue channels
  uint32_t rxb = bgc & 0xF81F;
  rxb += ((fgc & 0xF81F) - rxb) * (alpha >> 2) >> 6;
  // Split out and blend 6 bit green channel
  uint32_t xgx = bgc & 0x07E0;
  xgx += ((fgc & 0x07E0) - xgx) * alpha >> 8;
  // Recombine channels
  return (rxb & 0xF81F) | (xgx & 0x07E0);
}

/***************************************************************************************
** Function name:           alphaBlend
** Description:             Blend 16bit foreground and background with dither
*************************************************************************************x*/
uint16_t TFT_GFX::alphaBlend(uint8_t alpha, uint16_t fgc, uint16_t bgc, uint8_t dither)
{
  if (dither) {
    int16_t alphaDither = (int16_t)alpha - dither + random(2*dither+1); // +/-4 randomised
    alpha = (uint8_t)alphaDither;
    if (alphaDither <  0) alpha = 0;
    if (alphaDither >255) alpha = 255;
  }

  return alphaBlend(alpha, fgc, bgc);
}

/***************************************************************************************
** Function name:           alphaBlend
** Description:             Blend 24bit foreground and background with optional dither
*************************************************************************************x*/
uint32_t TFT_GFX::alphaBlend24(uint8_t alpha, uint32_t fgc, uint32_t bgc, uint8_t dither)
{

  if (dither) {
    int16_t alphaDither = (int16_t)alpha - dither + random(2*dither+1); // +/-dither randomised
    alpha = (uint8_t)alphaDither;
    if (alphaDither <  0) alpha = 0;
    if (alphaDither >255) alpha = 255;
  }

  uint32_t rxx = bgc & 0xFF0000;
  rxx += ((fgc & 0xFF0000) - rxx) * alpha >> 8;
  uint32_t xgx = bgc & 0x00FF00;
  xgx += ((fgc & 0xFF0000) - xgx) * alpha >> 8;
  uint32_t xxb = bgc & 0x0000FF;
  xxb += ((fgc & 0xFF0000) - xxb) * alpha >> 8;
  return (rxx & 0xFF0000) | (xgx & 0x00FF00) | (xxb & 0x0000FF);
}


/*
  Sprites adaptation to tsdeeSPI

  Copyright (c) 2024, rspber (https://github.com/rspber)

  Based on: TFT_eSPI

  Originally notes below:
*/

/**************************************************************************************
// The following class creates Sprites in RAM, graphics can then be drawn in the Sprite
// and rendered quickly onto the TFT screen. The class inherits the graphics functions
// from the TFT_eSPI class. Some functions are overridden by this class so that the
// graphics are written to the Sprite rather than the TFT.
// Coded by Bodmer, see license file in root folder
***************************************************************************************/
/***************************************************************************************
// Color bytes are swapped when writing to RAM, this introduces a small overhead but
// there is a nett performance gain by using swapped bytes.
***************************************************************************************/

/***************************************************************************************
** Function name:           TFT_eSprite
** Description:             Class constructor
***************************************************************************************/
TFT_eSprite::TFT_eSprite(TFT_eSPI *tft) : TFT_eSPI()
{
  _tft = tft;     // Pointer to tft class so we can call member functions

  _iwidth    = 0; // Initialise width and height to 0 (it does not exist yet)
  _iheight   = 0;
  _bpp = 16;
  _swapBytes = false;   // Do not swap pushImage colour bytes by default

  _created = false;
  _clip.x1 = 1;
  _clip.x2 = 0;

  _xs = 0;  // window bounds for pushColor
  _ys = 0;
  _xe = 0;
  _ye = 0;

  _xptr = 0; // pushColor coordinate
  _yptr = 0;

  _cmap.cMap = nullptr;
  _cmap.len = 0;

  _psram_enable = true;
  
  // Ensure end_tft_write() does nothing in inherited functions.
  lockTransaction = true;
}


/***************************************************************************************
** Function name:           createSprite
** Description:             Create a sprite (bitmap) of defined width and height
***************************************************************************************/
// cast returned value to (uint8_t*) for 8-bit or (uint16_t*) for 16-bit colours
void* TFT_eSprite::createSprite(int16_t w, int16_t h, uint8_t frames)
{
  Serial.println("createSprite");

  if ( _created ) return _img8_1;

  if ( w < 1 || h < 1 ) return nullptr;

  _iwidth  = _dwidth  = _bitwidth = w;
  _iheight = _dheight = h;

  _cursor.x = 0;
  _cursor.y = 0;

  // Default scroll rectangle and gap fill colour
  _sx = 0;
  _sy = 0;
  _sw = w;
  _sh = h;
  _scolor = TFT_BLACK;

  _img8   = (uint8_t*) callocSprite(w, h, frames);
  _img8_1 = _img8;
  _img8_2 = _img8;
  _img    = (uint16_t*) _img8;
  _img4   = _img8;

  if ( (_bpp == 16) && (frames > 1) ) {
    _img8_2 = _img8 + (w * h * 2 + 1);
  }

  // ESP32 only 16bpp check
  //if (esp_ptr_dma_capable(_img8_1)) Serial.println("DMA capable Sprite pointer _img8_1");
  //else Serial.println("Not a DMA capable Sprite pointer _img8_1");
  //if (esp_ptr_dma_capable(_img8_2)) Serial.println("DMA capable Sprite pointer _img8_2");
  //else Serial.println("Not a DMA capable Sprite pointer _img8_2");

  if ( (_bpp == 8) && (frames > 1) ) {
    _img8_2 = _img8 + (w * h + 1);
  }

  // This is to make it clear what pointer size is expected to be used
  // but casting in the user sketch is needed due to the use of void*
  if ( (_bpp == 1) && (frames > 1) ) {
    w = (w+7) & 0xFFF8;
    _img8_2 = _img8 + ( (w>>3) * h + 1 );
  }

  if (_img8) {
    _created = true;
    if ( (_bpp == 4) && (_cmap.cMap == nullptr)) createPalette(default_4bit_palette);

    rotation = 0;
    setViewport(0, 0, _dwidth, _dheight);
    setPivot(_iwidth/2, _iheight/2);
    return _img8_1;
  }

  return nullptr;
}


/***************************************************************************************
** Function name:           getPointer
** Description:             Returns pointer to start of sprite memory area
***************************************************************************************/
void* TFT_eSprite::getPointer(void)
{
  if (!_created) return nullptr;
  return _img8_1;
}


/***************************************************************************************
** Function name:           created
** Description:             Returns true if sprite has been created
***************************************************************************************/
bool TFT_eSprite::created(void)
{
  return _created;
}


/***************************************************************************************
** Function name:           ~TFT_eSprite
** Description:             Class destructor
***************************************************************************************/
TFT_eSprite::~TFT_eSprite(void)
{
  deleteSprite();

#ifdef SMOOTH_FONT
  if(fontLoaded) unloadFont();
#endif
}


/***************************************************************************************
** Function name:           callocSprite
** Description:             Allocate a memory area for the Sprite and return pointer
***************************************************************************************/
void* TFT_eSprite::callocSprite(int16_t w, int16_t h, uint8_t frames)
{
  Serial.println("callocSprite");

  // Add one extra "off screen" pixel to point out-of-bounds setWindow() coordinates
  // this means push/writeColor functions do not need additional bounds checks and
  // hence will run faster in normal circumstances.
  uint8_t* ptr8 = nullptr;

  if (frames > 2) frames = 2; // Currently restricted to 2 frame buffers
  if (frames < 1) frames = 1;

  if (_bpp == 16)
  {
#if defined (ESP32) && defined (CONFIG_SPIRAM_SUPPORT)
    if ( psramFound() && _psram_enable && !_tft->DMA_Enabled)
    {
      ptr8 = ( uint8_t*) ps_calloc(frames * w * h + frames, sizeof(uint16_t));
      //Serial.println("PSRAM");
    }
    else
#endif
    {
      ptr8 = ( uint8_t*) calloc(frames * w * h + frames, sizeof(uint16_t));
      //Serial.println("Normal RAM");
    }
  }

  else if (_bpp == 8)
  {
#if defined (ESP32) && defined (CONFIG_SPIRAM_SUPPORT)
    if ( psramFound() && _psram_enable ) ptr8 = ( uint8_t*) ps_calloc(frames * w * h + frames, sizeof(uint8_t));
    else
#endif
    ptr8 = ( uint8_t*) calloc(frames * w * h + frames, sizeof(uint8_t));
  }

  else if (_bpp == 4)
  {
    w = (w+1) & 0xFFFE; // width needs to be multiple of 2, with an extra "off screen" pixel
    _iwidth = w;
#if defined (ESP32) && defined (CONFIG_SPIRAM_SUPPORT)
    if ( psramFound() && _psram_enable ) ptr8 = ( uint8_t*) ps_calloc(((frames * w * h) >> 1) + frames, sizeof(uint8_t));
    else
#endif
    ptr8 = ( uint8_t*) calloc(((frames * w * h) >> 1) + frames, sizeof(uint8_t));
  }

  else // Must be 1 bpp
  {
    //_dwidth   Display width+height in pixels always in rotation 0 orientation
    //_dheight  Not swapped for sprite rotations
    // Note: for 1bpp _iwidth and _iheight are swapped during Sprite rotations

    w =  (w+7) & 0xFFF8; // width should be the multiple of 8 bits to be compatible with epdpaint
    _iwidth = w;         // _iwidth is rounded up to be multiple of 8, so might not be = _dwidth
    _bitwidth = w;       // _bitwidth will not be rotated whereas _iwidth may be

#if defined (ESP32) && defined (CONFIG_SPIRAM_SUPPORT)
    if ( psramFound() && _psram_enable ) ptr8 = ( uint8_t*) ps_calloc(frames * (w>>3) * h + frames, sizeof(uint8_t));
    else
#endif
    ptr8 = ( uint8_t*) calloc(frames * (w>>3) * h + frames, sizeof(uint8_t));
  }

  return ptr8;
}


/***************************************************************************************
** Function name:           createPalette (from RAM array)
** Description:             Set a palette for a 4-bit per pixel sprite
***************************************************************************************/
void TFT_eSprite::createPalette(rgb_t colorMap[], uint8_t colors)
{
  Serial.println("createPalette 1");

  if (!_created) return;

  if (colorMap == nullptr) {
    // Create a color map using the default FLASH map
    createPalette(default_4bit_palette);
    return;
  }

  if (colors > 16) colors = 16;
  _cmap.len = colors;
  _cmap.rLen = 0;

  // Allocate and clear memory for 16 color map
  _cmap.cMap = (rgb_t *)realloc(_cmap.cMap, colors*3 * sizeof(rgb_t));

  // Copy map colors
  for (int i = 0; i < colors; i++) {
    _cmap.cMap[i] = colorMap[i];
  }
}


/***************************************************************************************
** Function name:           createPalette (from FLASH array)
** Description:             Set a palette for a 4-bit per pixel sprite
***************************************************************************************/
void TFT_eSprite::createPalette(const rgb_t colorMap[], uint8_t colors)
{
  Serial.println("createPalette 2");

  if (!_created) return;

  if (colorMap == nullptr)
  {
    // Create a color map using the default FLASH map
    colorMap = default_4bit_palette;
  }

  if (colors > 16) colors = 16;
  _cmap.len = colors;
  _cmap.rLen = 0;

  // Allocate and clear memory for 16 color map
  _cmap.cMap = (rgb_t *)realloc(_cmap.cMap, colors*3 * sizeof(rgb_t));

  // Copy map colors
  for (uint8_t i = 0; i < colors; i++) {
    _cmap.cMap[i] = color16to24(pgm_read_word(colorMap++));
  }
}


/***************************************************************************************
** Function name:           frameBuffer
** Description:             For 1 bpp Sprites, select the frame used for graphics
***************************************************************************************/
// Frames are numbered 1 and 2
void* TFT_eSprite::frameBuffer(int8_t f)
{
  Serial.println("frameBuffer");

  if (!_created) return nullptr;

  if ( f == 2 ) _img8 = _img8_2;
  else          _img8 = _img8_1;

  if (_bpp == 16) _img = (uint16_t*)_img8;

  //if (_bpp == 8) _img8 = _img8;

  if (_bpp == 4) _img4 = _img8;

  return _img8;
}


/***************************************************************************************
** Function name:           setColorDepth
** Description:             Set bits per pixel for colour (1, 8 or 16)
***************************************************************************************/
void* TFT_eSprite::setColorDepth(int8_t b)
{
  Serial.println("setColorDepth");

  // Do not re-create the sprite if the colour depth does not change
  if (_bpp == b) return _img8_1;

  // Validate the new colour depth
  if ( b > 8 ) _bpp = 16;  // Bytes per pixel
  else if ( b > 4 ) _bpp = 8;
  else if ( b > 1 ) _bpp = 4;
  else _bpp = 1;

  // Can't change an existing sprite's colour depth so delete and create a new one
  if (_created) {
    deleteSprite();
    return createSprite(_dwidth, _dheight);
  }

  return nullptr;
}


/***************************************************************************************
** Function name:           getColorDepth
** Description:             Get bits per pixel for colour (1, 8 or 16)
***************************************************************************************/
int8_t TFT_eSprite::getColorDepth(void)
{
  Serial.println("getColorDepth");

  if (_created) return _bpp;
  else return 0;
}


/***************************************************************************************
** Function name:           setBitmapColor
** Description:             Set the 1bpp foreground foreground and background colour
***************************************************************************************/
void TFT_eSprite::setBitmapColor(rgb_t c, rgb_t b)
{
  Serial.println("setBitmapColor");

  if (c == b) b = ~c;
  _tft->_bitmap_fg = c;
  _tft->_bitmap_bg = b;
}


/***************************************************************************************
** Function name:           setPaletteColor
** Description:             Set the 4bpp palette color at the given index
***************************************************************************************/
void TFT_eSprite::setPaletteColor(uint8_t index, rgb_t color)
{
  Serial.println("setPaletteColor");

  if (_cmap.cMap == nullptr || index >= _cmap.len) return; // out of bounds

  _cmap.cMap[index] = color;
}


/***************************************************************************************
** Function name:           getPaletteColor
** Description:             Return the palette color at 4bpp index, or 0 on error.
***************************************************************************************/
uint16_t TFT_eSprite::getPaletteColor(uint8_t index)
{
  Serial.println("getPaletteColor");

  if (_cmap.cMap == nullptr || index >= _cmap.len) return 0; // out of bounds

  return _cmap.cMap[index];
}


/***************************************************************************************
** Function name:           deleteSprite
** Description:             Delete the sprite to free up memory (RAM)
***************************************************************************************/
void TFT_eSprite::deleteSprite(void)
{
  Serial.println("deleteSprite");

  if (_cmap.cMap != nullptr) {
    free(_cmap.cMap);
    _cmap.cMap = nullptr;
  }
  _cmap.len = 0;
  _cmap.rLen = 0;

  if (_created) {
    free(_img8_1);
    _img8 = nullptr;
    _created = false;
    _clip.x1 = 1;
    _clip.x2 = 0;
  }
}


/***************************************************************************************
** Function name:           pushRotated - Fast fixed point integer maths version
** Description:             Push rotated Sprite to TFT screen
***************************************************************************************/
#define FP_SCALE 10
bool TFT_eSprite::pushRotated(int16_t angle, rgb_t transp)
{
  Serial.println("pushRotated 1");

  if ( !_created) return false;

  // Bounding box parameters
  int16_t min_x;
  int16_t min_y;
  int16_t max_x;
  int16_t max_y;

  // Get the bounding box of this rotated source Sprite relative to Sprite pivot
  if ( !getRotatedBounds(angle, &min_x, &min_y, &max_x, &max_y) ) return false;

  uint16_t sline_buffer[max_x - min_x + 1];

  int32_t xt = min_x - _tft->_xPivot;
  int32_t yt = min_y - _tft->_yPivot;
  uint32_t xe = _dwidth << FP_SCALE;
  uint32_t ye = _dheight << FP_SCALE;

  _tft->startWrite(); // Avoid transaction overhead for every tft pixel

  // Scan destination bounding box and fetch transformed pixels from source Sprite
  for (int32_t y = min_y; y <= max_y; y++, yt++) {
    int32_t x = min_x;
    uint32_t xs = (_cosra * xt - (_sinra * yt - (_xPivot << FP_SCALE)) + (1 << (FP_SCALE - 1)));
    uint32_t ys = (_sinra * xt + (_cosra * yt + (_yPivot << FP_SCALE)) + (1 << (FP_SCALE - 1)));

    while ((xs >= xe || ys >= ye) && x < max_x) { x++; xs += _cosra; ys += _sinra; }
    if (x == max_x) continue;

    uint32_t pixel_count = 0;
    do {
      int32_t xp = xs >> FP_SCALE;
      int32_t yp = ys >> FP_SCALE;
      rgb_t rp;
      if (_bpp == 16) {
        rp = color16to24(_img[xp + yp * _iwidth]);
      }
      else {
        rp = readPixel(_clip, xp, yp);
      }
      if (!eqColorsFC(transp, WHITE) && eqColorsFC(transp, rp)) {
        if (pixel_count) {
          // TFT window is already clipped, so this is faster than pushImage
          _tft->setWindow(x - pixel_count, y, pixel_count, 1);
          _tft->pushPixels16(sline_buffer, pixel_count);
          pixel_count = 0;
        }
      }
      else {
        sline_buffer[pixel_count++] = color24to16(rp);
      }
    } while (++x < max_x && (xs += _cosra) < xe && (ys += _sinra) < ye);
    if (pixel_count) {
      // TFT window is already clipped, so this is faster than pushImage
      _tft->setWindow(x - pixel_count, y, pixel_count, 1);
      _tft->pushPixels16(sline_buffer, pixel_count);
    }
  }

  _tft->endWrite(); // End transaction

  return true;
}


/***************************************************************************************
** Function name:           pushRotated - Fast fixed point integer maths version
** Description:             Push a rotated copy of the Sprite to another Sprite
***************************************************************************************/
// Not compatible with 4bpp
bool TFT_eSprite::pushRotated(TFT_eSprite *spr, int16_t angle, rgb_t transp)
{
  Serial.println("pushRotated 2");

  if ( !_created  || _bpp == 4) return false; // Check this Sprite is created
  if ( !spr->_created  || spr->_bpp == 4) return false;  // Ckeck destination Sprite is created

  // Bounding box parameters
  int16_t min_x;
  int16_t min_y;
  int16_t max_x;
  int16_t max_y;

  // Get the bounding box of this rotated source Sprite
  if ( !getRotatedBounds(spr, angle, &min_x, &min_y, &max_x, &max_y) ) return false;

  uint16_t sline_buffer[max_x - min_x + 1];

  int32_t xt = min_x - spr->_xPivot;
  int32_t yt = min_y - spr->_yPivot;
  uint32_t xe = _dwidth << FP_SCALE;
  uint32_t ye = _dheight << FP_SCALE;
  
  bool oldSwapBytes = spr->getSwapBytes();
  spr->setSwapBytes(false);

  // Scan destination bounding box and fetch transformed pixels from source Sprite
  for (int32_t y = min_y; y <= max_y; y++, yt++) {
    int32_t x = min_x;
    uint32_t xs = (_cosra * xt - (_sinra * yt - (_xPivot << FP_SCALE)) + (1 << (FP_SCALE - 1)));
    uint32_t ys = (_sinra * xt + (_cosra * yt + (_yPivot << FP_SCALE)) + (1 << (FP_SCALE - 1)));

    while ((xs >= xe || ys >= ye) && x < max_x) { x++; xs += _cosra; ys += _sinra; }
    if (x == max_x) continue;

    uint32_t pixel_count = 0;
    do {
      int32_t xp = xs >> FP_SCALE;
      int32_t yp = ys >> FP_SCALE;
      rgb_t rp;
      if (_bpp == 16) {
        rp = color16to24(_img[xp + yp * _iwidth]);
      }
      else {
        rp = readPixel(_clip, xp, yp);
      }
      if (!eqColorsFC(transp, WHITE) && eqColorsFC(transp, rp)) {
        if (pixel_count) {
          spr->pushImage16(x - pixel_count, y, pixel_count, 1, sline_buffer);
          pixel_count = 0;
        }
      }
      else {
        sline_buffer[pixel_count++] = color24to16(rp);
      }
    } while (++x < max_x && (xs += _cosra) < xe && (ys += _sinra) < ye);
    if (pixel_count) spr->pushImage16(x - pixel_count, y, pixel_count, 1, sline_buffer);
  }
  spr->setSwapBytes(oldSwapBytes);
  return true;
}


/***************************************************************************************
** Function name:           getRotatedBounds
** Description:             Get TFT bounding box of a rotated Sprite wrt pivot
***************************************************************************************/
bool TFT_eSprite::getRotatedBounds(int16_t angle, int16_t *min_x, int16_t *min_y,
                                                  int16_t *max_x, int16_t *max_y)
{
//  Serial.println("getRotatedBounds 1");

  // Get the bounding box of this rotated source Sprite relative to Sprite pivot
  getRotatedBounds(angle, width(), height(), _xPivot, _yPivot, min_x, min_y, max_x, max_y);

  // Move bounding box so source Sprite pivot coincides with TFT pivot
  *min_x += _tft->_xPivot;
  *max_x += _tft->_xPivot;
  *min_y += _tft->_yPivot;
  *max_y += _tft->_yPivot;

  // Return if bounding box is outside of TFT viewport
  if (*min_x > _tft->_clip.x2) return false;
  if (*min_y > _tft->_clip.y2) return false;
  if (*max_x < _tft->_clip.x1) return false;
  if (*max_y < _tft->_clip.y1) return false;

  // Clip bounding box to be within TFT viewport
  if (*min_x < _tft->_clip.x1) *min_x = _tft->_clip.x1;
  if (*min_y < _tft->_clip.y1) *min_y = _tft->_clip.y1;
  if (*max_x > _tft->_clip.x2) *max_x = _tft->_clip.x2;
  if (*max_y > _tft->_clip.y2) *max_y = _tft->_clip.y2;

  return true;
}


/***************************************************************************************
** Function name:           getRotatedBounds
** Description:             Get destination Sprite bounding box of a rotated Sprite wrt pivot
***************************************************************************************/
bool TFT_eSprite::getRotatedBounds(TFT_eSprite *spr, int16_t angle, int16_t *min_x, int16_t *min_y,
                                                                    int16_t *max_x, int16_t *max_y)
{
//  Serial.println("getRotatedBounds 2");

  // Get the bounding box of this rotated source Sprite relative to Sprite pivot
  getRotatedBounds(angle, width(), height(), _xPivot, _yPivot, min_x, min_y, max_x, max_y);

  // Move bounding box so source Sprite pivot coincides with destination Sprite pivot
  *min_x += spr->_xPivot;
  *max_x += spr->_xPivot;
  *min_y += spr->_yPivot;
  *max_y += spr->_yPivot;

  // Test only to show bounding box
  // spr->fillSprite(TFT_BLACK);
  // spr->drawRect(min_x, min_y, max_x - min_x + 1, max_y - min_y + 1, TFT_GREEN);

  // Return if bounding box is completely outside of destination Sprite
  if (*min_x > spr->width()) return true;
  if (*min_y > spr->height()) return true;
  if (*max_x < 0) return true;
  if (*max_y < 0) return true;

  // Clip bounding box to Sprite boundaries
  // Clipping to a viewport will be done by destination Sprite pushImage function
  if (*min_x < 0) min_x = 0;
  if (*min_y < 0) min_y = 0;
  if (*max_x > spr->width())  *max_x = spr->width();
  if (*max_y > spr->height()) *max_y = spr->height();

  return true;
}


/***************************************************************************************
** Function name:           rotatedBounds
** Description:             Get bounding box of a rotated Sprite wrt pivot
***************************************************************************************/
void TFT_eSprite::getRotatedBounds(int16_t angle, int16_t w, int16_t h, int16_t xp, int16_t yp,
                                   int16_t *min_x, int16_t *min_y, int16_t *max_x, int16_t *max_y)
{
//  Serial.println("getRotatedBounds 3");

  // Trig values for the rotation
  float radAngle = -angle * 0.0174532925; // Convert degrees to radians
  float sina = sin(radAngle);
  float cosa = cos(radAngle);

  w -= xp; // w is now right edge coordinate relative to xp
  h -= yp; // h is now bottom edge coordinate relative to yp

  // Calculate new corner coordinates
  int16_t x0 = -xp * cosa - yp * sina;
  int16_t y0 =  xp * sina - yp * cosa;

  int16_t x1 =  w * cosa - yp * sina;
  int16_t y1 = -w * sina - yp * cosa;

  int16_t x2 =  h * sina + w * cosa;
  int16_t y2 =  h * cosa - w * sina;

  int16_t x3 =  h * sina - xp * cosa;
  int16_t y3 =  h * cosa + xp * sina;

  // Find bounding box extremes, enlarge box to accomodate rounding errors
  *min_x = x0-2;
  if (x1 < *min_x) *min_x = x1-2;
  if (x2 < *min_x) *min_x = x2-2;
  if (x3 < *min_x) *min_x = x3-2;

  *max_x = x0+2;
  if (x1 > *max_x) *max_x = x1+2;
  if (x2 > *max_x) *max_x = x2+2;
  if (x3 > *max_x) *max_x = x3+2;

  *min_y = y0-2;
  if (y1 < *min_y) *min_y = y1-2;
  if (y2 < *min_y) *min_y = y2-2;
  if (y3 < *min_y) *min_y = y3-2;

  *max_y = y0+2;
  if (y1 > *max_y) *max_y = y1+2;
  if (y2 > *max_y) *max_y = y2+2;
  if (y3 > *max_y) *max_y = y3+2;

  _sinra = round(sina * (1<<FP_SCALE));
  _cosra = round(cosa * (1<<FP_SCALE));
}


/***************************************************************************************
** Function name:           pushSprite
** Description:             Push the sprite to the TFT at x, y
***************************************************************************************/
void TFT_eSprite::pushSprite(int32_t x, int32_t y)
{
  Serial.println("pushSprite 1");

  if (!_created) return;

  if (_bpp == 16) {
    bool oldSwapBytes = _tft->getSwapBytes();
    _tft->setSwapBytes(false);
    _tft->pushImage16(x, y, _dwidth, _dheight, _img );
    _tft->setSwapBytes(oldSwapBytes);
  }
  else if (_bpp == 4) {
    _tft->pushImage16(x, y, _dwidth, _dheight, _img4, false, _cmap);
  }
  else {
    cmap_t tmp = {0};
    _tft->pushImage16(x, y, _dwidth, _dheight, _img8, (bool)(_bpp == 8), tmp);
  }
}


/***************************************************************************************
** Function name:           pushSprite
** Description:             Push the sprite to the TFT at x, y with transparent colour
***************************************************************************************/
void TFT_eSprite::pushSprite(int32_t x, int32_t y, rgb_t transp)
{
  Serial.println("pushSprite 2");

  if (!_created) return;

  if (_bpp == 16) {
    bool oldSwapBytes = _tft->getSwapBytes();
    _tft->setSwapBytes(false);
    _tft->pushImage16(x, y, _dwidth, _dheight, _img, transp);
    _tft->setSwapBytes(oldSwapBytes);
  }
  else if (_bpp == 8) {
    cmap_t tmp = {0};
    _tft->pushImage16(x, y, _dwidth, _dheight, _img8, transp, (bool)true, tmp);
  }
  else if (_bpp == 4) {
    _tft->pushImage16(x, y, _dwidth, _dheight, _img4, transp, false, _cmap);
  }
  else {
    cmap_t tmp = {0};
    _tft->pushImage16(x, y, _dwidth, _dheight, _img8, 0, (bool)false, tmp);
  }
}


/***************************************************************************************
** Function name:           pushToSprite
** Description:             Push the sprite to another sprite at x, y
***************************************************************************************/
// Note: The following sprite to sprite colour depths are currently supported:
//    Source    Destination
//    16bpp  -> 16bpp
//    16bpp  ->  8bpp
//     8bpp  ->  8bpp
//     4bpp  ->  4bpp (note: color translation depends on the 2 sprites palette colors)
//     1bpp  ->  1bpp (note: color translation depends on the 2 sprites bitmap colors)

bool TFT_eSprite::pushToSprite(TFT_eSprite *dspr, int32_t x, int32_t y)
{
  Serial.println("pushToSprite 1");

  if (!_created) return false;
  if (!dspr->created()) return false;

  // Check destination sprite compatibility
  int8_t ds_bpp = dspr->getColorDepth();
  if (_bpp == 16 && ds_bpp != 16 && ds_bpp !=  8) return false;
  if (_bpp ==  8 && ds_bpp !=  8) return false;
  if (_bpp ==  4 && ds_bpp !=  4) return false;
  if (_bpp ==  1 && ds_bpp !=  1) return false;

  bool oldSwapBytes = dspr->getSwapBytes();
  dspr->setSwapBytes(false);
  dspr->pushImage16(x, y, _dwidth, _dheight, _img, _bpp);
  dspr->setSwapBytes(oldSwapBytes);

  return true;
}


/***************************************************************************************
** Function name:           pushToSprite
** Description:             Push the sprite to another sprite at x, y with transparent colour
***************************************************************************************/
// Note: The following sprite to sprite colour depths are currently supported:
//    Source    Destination
//    16bpp  -> 16bpp
//    16bpp  ->  8bpp
//     8bpp  ->  8bpp
//     1bpp  ->  1bpp

bool TFT_eSprite::pushToSprite(TFT_eSprite *dspr, int32_t x, int32_t y, rgb_t transp)
{
  Serial.println("pushToSprite 2");

  if ( !_created  || !dspr->_created) return false; // Check Sprites exist

  // Check destination sprite compatibility
  int8_t ds_bpp = dspr->getColorDepth();
  if (_bpp == 16 && ds_bpp != 16 && ds_bpp !=  8) return false;
  if (_bpp ==  8 && ds_bpp !=  8) return false;
  if (_bpp ==  4 || ds_bpp ==  4) return false;
  if (_bpp ==  1 && ds_bpp !=  1) return false;

  bool oldSwapBytes = dspr->getSwapBytes();
  uint16_t sline_buffer[width()];

  // Scan destination bounding box and fetch transformed pixels from source Sprite
  for (int32_t ys = 0; ys < height(); ys++) {
    int32_t ox = x;
    uint32_t pixel_count = 0;

    for (int32_t xs = 0; xs < width(); xs++) {
      rgb_t rp = 0;
      if (_bpp == 16) {
        rp = color16to24(_img[xs + ys * width()]);
      }
      else {
        rp = readPixel(_clip, xs, ys);
      }
      //dspr->drawPixel(xs, ys, rp);

      if (eqColorsFC(transp, rp) ) {
        if (pixel_count) {
          dspr->pushImage16(ox, y, pixel_count, 1, sline_buffer);
          ox += pixel_count;
          pixel_count = 0;
        }
        ox++;
      }
      else {
        sline_buffer[pixel_count++] = color24to16(rp);
      }
    }
    if (pixel_count) dspr->pushImage16(ox, y, pixel_count, 1, sline_buffer);
    y++;
  }
  dspr->setSwapBytes(oldSwapBytes);
  return true;
}


/***************************************************************************************
** Function name:           pushSprite
** Description:             Push a cropped sprite to the TFT at tx, ty
***************************************************************************************/
bool TFT_eSprite::pushSprite(int32_t tx, int32_t ty, int32_t sx, int32_t sy, int32_t sw, int32_t sh)
{
  Serial.println("pushSprite");

  if (!_created) return false;

  // Perform window boundary checks and crop if needed
  setWindow(sx, sy, sw, sh);

  /* These global variables are now populated for the sprite
  _xs = x start coordinate
  _ys = y start coordinate
  _xe = x end coordinate (inclusive)
  _ye = y end coordinate (inclusive)
  */

  // Calculate new sprite window bounding box width and height
  sw = _xe - _xs + 1;
  sh = _ye - _ys + 1;

  if (_ys >= _iheight) return false;

  if (_bpp == 16) {
    bool oldSwapBytes = _tft->getSwapBytes();
    _tft->setSwapBytes(false);

    // Check if a faster block copy to screen is possible
    if ( sx == 0 && sw == _dwidth)
      _tft->pushImage16(tx, ty, sw, sh, _img + _iwidth * _ys );
    else // Render line by line
      while (sh--)
        _tft->pushImage16(tx, ty++, sw, 1, _img + _xs + _iwidth * _ys++ );

    _tft->setSwapBytes(oldSwapBytes);
  }
  else if (_bpp == 8) {
    // Check if a faster block copy to screen is possible
    if ( sx == 0 && sw == _dwidth) {
      cmap_t tmp = {0};
      _tft->pushImage16(tx, ty, sw, sh, _img8 + _iwidth * _ys, (bool)true, tmp);
    }
    else // Render line by line
    while (sh--) {
      cmap_t tmp = {0};
      _tft->pushImage16(tx, ty++, sw, 1, _img8 + _xs + _iwidth * _ys++, (bool)true, tmp);
    }
  }
  else if (_bpp == 4) {
    // Check if a faster block copy to screen is possible
    if ( sx == 0 && sw == _dwidth) {
      _tft->pushImage16(tx, ty, sw, sh, _img4 + (_iwidth>>1) * _ys, false, _cmap );
    }
    else { // Render line by line
      int32_t ds = _xs&1; // Odd x start pixel

      int32_t de = 0;     // Odd x end pixel
      if ((sw > ds) && (_xe&1)) de = 1;

      uint32_t dm = 0;     // Midsection pixel count
      if (sw > (ds+de)) dm = sw - ds - de;
      sw--;

      uint32_t yp = (_xs + ds + _iwidth * _ys)>>1;
      _tft->startWrite();
      while (sh--) {
        if (ds) _tft->drawPixel(tx, ty, readPixel(_clip, _xs, _ys) );
        if (dm) _tft->pushImage16(tx + ds, ty, dm, 1, _img4 + yp, false, _cmap );
        if (de) _tft->drawPixel(tx + sw, ty, readPixel(_clip, _xe, _ys) );
        _ys++;
        ty++;
        yp += (_iwidth>>1);
      }
      _tft->endWrite();
    }
  }
  else { // 1bpp
    // Check if a faster block copy to screen is possible
    if ( sx == 0 && sw == _dwidth) {
      cmap_t tmp = {0};
      _tft->pushImage16(tx, ty, sw, sh, _img8 + (_bitwidth>>3) * _ys, (bool)false, tmp);
    }
    else { // Render line by line
      _tft->startWrite();
      while (sh--) {
        cmap_t tmp = {0};
        _tft->pushImage16(tx, ty++, sw, 1, _img8 + (_bitwidth>>3) * _ys++, (bool)false, tmp);
      }
      _tft->endWrite();
    }
  }

  return true;
}


/***************************************************************************************
** Function name:           readPixelValue
** Description:             Read the color map index of a pixel at defined coordinates
***************************************************************************************/
rgb_t TFT_eSprite::readPixelValue(int32_t x, int32_t y)
{
//  Serial.println("readPixelValue");

  if (!_created) return TFT_BLACK;

  x+= _clip.xDatum;
  y+= _clip.yDatum;

  // Range checking
  if ((x < _clip.x1) || (y < _clip.y1) ||(x >= _clip.x2) || (y >= _clip.y2)) return 0xFF;

  if (_bpp == 16) {
    // Return the pixel colour
    return readPixel(_clip, x - _clip.xDatum, y - _clip.yDatum);
  }

  if (_bpp == 8) {
    // Return the pixel byte value
    uint8_t c8 = _img8[x + y * _iwidth];
    rgb_t c24 = color8to24(c8);
    Serial.printf(" c8:%02x=%08x", c8, c24);
    return c24;
  }

  if (_bpp == 4) {
    if (x >= _dwidth) return TFT_WHITE;
    if ((x & 0x01) == 0)
      return _cmap.cMap[_img4[((x+y*_iwidth)>>1)] >> 4];   // even index = bits 7 .. 4
    else
      return _cmap.cMap[_img4[((x+y*_iwidth)>>1)] & 0x0F]; // odd index = bits 3 .. 0.
  }

  if (_bpp == 1) {
    // Note: _dwidth and _dheight bounds not checked (rounded up -iwidth and _iheight used)
    if (rotation == 1) {
      uint16_t tx = x;
      x = _dheight - y - 1;
      y = tx;
    }
    else if (rotation == 2) {
      x = _dwidth - x - 1;
      y = _dheight - y - 1;
    }
    else if (rotation == 3) {
      uint16_t tx = x;
      x = y;
      y = _dwidth - tx - 1;
    }
    // Return 1 or 0
    return _cmap.cMap[(_img8[(x + y * _bitwidth)>>3] >> (7-(x & 0x7))) & 0x01];
  }

  return 0;
}

namespace {
  uint8_t  blue[] = {0, 0x5F, 0xAF, 0xFF};
};

/***************************************************************************************
** Function name:           readPixel
** Description:             Read 565 colour of a pixel at defined coordinates
***************************************************************************************/
rgb_t TFT_eSprite::readPixel(clip_t& clip, int32_t x, int32_t y)
{
//  Serial.println("readPixel");

  if (!_created) return 0xFFFF;

  x+= clip.xDatum;
  y+= clip.yDatum;

  // Range checking
  if ((x < clip.x1) || (y < clip.y1) ||(x >= clip.x2) || (y >= clip.y2)) return TFT_WHITE;

  if (_bpp == 16) {
    return color16to24(_img[x + y * _iwidth]);
  }

  if (_bpp == 8) {
    uint8_t c8 = _img8[x + y * _iwidth];
    if (c8 != 0) {
      uint32_t c24 = (c8 & 0xE0)<<16 | (c8 & 0xC0)<<13 | (c8 & 0x1C)<<11 | (c8 & 0x1C)<<8 | blue[c8 & 0x03];
      Serial.printf(" c8:%02x=%08x", c8, c24);
      return c24;
    }
    return TFT_BLACK;
  }

  if (_bpp == 4) {
    if (x >= _dwidth) return TFT_WHITE;
    if ((x & 0x01) == 0) {
      return _cmap.cMap[_img4[((x+y*_iwidth)>>1)] >> 4];   // even index = bits 7 .. 4
    }
    else {
      return _cmap.cMap[_img4[((x+y*_iwidth)>>1)] & 0x0F]; // odd index = bits 3 .. 0.
    }
  }

  // Note: Must be 1bpp
  // _dwidth and _dheight bounds not checked (rounded up -iwidth and _iheight used)
  if (rotation == 1) {
    uint16_t tx = x;
    x = _dheight - y - 1;
    y = tx;
  }
  else if (rotation == 2) {
    x = _dwidth - x - 1;
    y = _dheight - y - 1;
  }
  else if (rotation == 3) {
    uint16_t tx = x;
    x = y;
    y = _dwidth - tx - 1;
  }

  uint8_t idx = (_img8[(x + y * _bitwidth)>>3] << (x & 0x7)) & 0x80;

  if (idx) return _tft->_bitmap_fg;
  else       return _tft->_bitmap_bg;
}


/***************************************************************************************
** Function name:           pushImage
** Description:             push image into a defined area of a sprite
***************************************************************************************/
void  TFT_eSprite::pushImage16(int32_t x, int32_t y, int32_t w, int32_t h, uint16_t *data, uint8_t sbpp)
{
  Serial.println("pushImage 1");

  if (data == nullptr || !_created) return;

  clip_t& clip = _clip;
  PI_CLIP;

  if (_bpp == 16) { // Plot a 16 bpp image into a 16 bpp Sprite
    // Pointer within original image
    uint8_t *ptro = (uint8_t *)data + ((dx + dy * w) << 1);
    // Pointer within sprite image
    uint8_t *ptrs = (uint8_t *)_img + ((x + y * _iwidth) << 1);

    if(_swapBytes) {
      while (dh--) {
        // Fast copy with a 1 byte shift
        memcpy(ptrs+1, ptro, (dw<<1) - 1);
        // Now correct just the even numbered bytes
        for (int32_t xp = 0; xp < (dw<<1); xp+=2) {
          ptrs[xp] = ptro[xp+1];;
        }
        ptro += w<<1;
        ptrs += _iwidth<<1;
      }
    }
    else {
      while (dh--) {
        memcpy(ptrs, ptro, dw<<1);
        ptro += w << 1;
        ptrs += _iwidth << 1;
      }
    }
  }
  else if (_bpp == 8 && sbpp == 8) { // Plot a 8 bpp image into a 8 bpp Sprite
    // Pointer within original image
    uint8_t *ptro = (uint8_t *)data + (dx + dy * w);
    // Pointer within sprite image
    uint8_t *ptrs = (uint8_t *)_img + (x + y * _iwidth);

    while (dh--) {
      memcpy(ptrs, ptro, dw);
      ptro += w;
      ptrs += _iwidth;
    }
  }
  else if (_bpp == 8) { // Plot a 16 bpp image into a 8 bpp Sprite
    for (int32_t yp = dy; yp < dy + dh; yp++) {
      int32_t xyw = x + y * _iwidth;
      int32_t dxypw = dx + yp * w;
      for (int32_t xp = dx; xp < dx + dw; xp++) {
        uint16_t color = data[dxypw++];
        _img8[xyw++] = color16to8(color);
      }
      y++;
    }
  }
  else if (_bpp == 4) {
    // The image is assumed to be 4-bit, where each byte corresponds to two pixels.
    // much faster when aligned to a byte boundary, because the alternative is slower, requiring
    // tedious bit operations.

    int sWidth = (_iwidth >> 1);
    uint8_t *ptr = (uint8_t *)data;

    if ((x & 0x01) == 0 && (dx & 0x01) == 0 && (dw & 0x01) == 0) {
      x = (x >> 1) + y * sWidth;
      dw = (dw >> 1);
      dx = (dx >> 1) + dy * (w>>1);
      while (dh--) {
        memcpy(_img4 + x, ptr + dx, dw);
        dx += (w >> 1);
        x += sWidth;
      }
    }
    else { // not optimized
      for (int32_t yp = dy; yp < dy + dh; yp++) {
        int32_t ox = x;
        for (int32_t xp = dx; xp < dx + dw; xp++) {
          int idx;
          if ((xp & 0x01) == 0)
            idx = (ptr[((xp+yp*w)>>1)] & 0xF0) >> 4; // even index = bits 7 .. 4
          else
            idx = ptr[((xp-1+yp*w)>>1)] & 0x0F;      // odd index = bits 3 .. 0.
          drawPixel(_clip, ox, y, _cmap.cMap[idx]);
          ox++;
        }
        y++;
      }
    }
  }

  else { // 1bpp
    // Plot a 1bpp image into a 1bpp Sprite
    uint32_t ww =  (w+7)>>3; // Width of source image line in bytes
    uint8_t *ptr = (uint8_t *)data;
    for (int32_t yp = dy;  yp < dy + dh; yp++) {
      uint32_t yw = yp * ww;              // Byte starting the line containing source pixel
      int32_t ox = x;
      for (int32_t xp = dx; xp < dx + dw; xp++) {
        int idx = (ptr[(xp>>3) + yw] & (0x80 >> (xp & 0x7)) );
        drawPixel(_clip, ox++, y, _cmap.cMap[idx]);
      }
      y++;
    }
  }
}


/***************************************************************************************
** Function name:           pushImage
** Description:             push 565 colour FLASH (PROGMEM) image into a defined area
***************************************************************************************/
void  TFT_eSprite::pushImage16(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data)
{
  Serial.println("pushImage 2");

#ifdef ESP32
  pushImage16(x, y, w, h, (uint16_t*) data);
#else
  // Partitioned memory FLASH processor
  if (data == nullptr || !_created) return;

  clip_t& clip = _clip;
  PI_CLIP;

  if (_bpp == 16) { // Plot a 16 bpp image into a 16 bpp Sprite
    for (int32_t yp = dy; yp < dy + dh; yp++) {
      int32_t ox = x;
      for (int32_t xp = dx; xp < dx + dw; xp++) {
        uint16_t color = pgm_read_word(data + xp + yp * w);
        if(_swapBytes) color = color<<8 | color>>8;
        _img[ox + y * _iwidth] = color;
        ox++;
      }
      y++;
    }
  }

  else if (_bpp == 8) { // Plot a 16 bpp image into a 8 bpp Sprite
    for (int32_t yp = dy; yp < dy + dh; yp++) {
      int32_t ox = x;
      for (int32_t xp = dx; xp < dx + dw; xp++) {
        uint16_t color = pgm_read_word(data + xp + yp * w);
        if(_swapBytes) color = color<<8 | color>>8;
        _img8[ox + y * _iwidth] = color16to8(color);
        ox++;
      }
      y++;
    }
  }

  else if (_bpp == 4) {
    #ifdef TFT_eSPI_DEBUG
    Serial.println("TFT_eSprite::pushImage16(int32_t x, int32_t y, int32_t w, int32_t h, const uint16_t *data) not implemented");
    #endif
    return;
  }

  else { // Plot a 1bpp image into a 1bpp Sprite
    x-= _clip.xDatum;   // Remove offsets, drawPixel will add
    y-= _clip.yDatum;
    uint16_t bsw =  (w+7) >> 3; // Width in bytes of source image line
    uint8_t *ptr = ((uint8_t*)data) + dy * bsw;
    
    while (dh--) {
      int32_t odx = dx;
      int32_t ox  = x;
      while (odx < dx + dw) {
        uint8_t pbyte = pgm_read_byte(ptr + (odx>>3));
        uint8_t mask = 0x80 >> (odx & 7);
        while (mask) {
          uint8_t p = pbyte & mask;
          mask = mask >> 1;
          drawPixel(_clip, ox++, y, _cmap.cMap[p != 0 ? 1 : 0]);
          odx++;
        }
      }
      ptr += bsw;
      y++;
    }
  }
#endif // if ESP32 check
}


/***************************************************************************************
** Function name:           setWindow
** Description:             Set the bounds of a window in the sprite
***************************************************************************************/
// Intentionally not constrained to viewport area, does not manage 1bpp rotations
void TFT_eSprite::setWindow(int32_t x1, int32_t y1, int32_t w, int32_t h)
{
//  Serial.println("setWindow");

  int32_t x2 = x1 + w - 1;
  int32_t y2 = y1 + h - 1;

  if (x1 > x2) transpose(x1, x2);
  if (y1 > y2) transpose(y1, y2);
  
  int32_t xover = width();
  int32_t yover = height();

  if ((x1 >= xover) || (x2 < 0) || (y1 >= yover) || (y2 < 0)) { // Point to that extra "off screen" pixel
    _xs = 0;
    _ys = _dheight;
    _xe = 0;
    _ye = _dheight;
  }
  else {
    _xs = x1 >= 0 ? x1 : 0;
    _ys = y1 >= 0 ? y1 : 0;
    _xe = x2 < xover ? x2 : xover - 1;
    _ye = y2 < yover ? y2 : yover - 1;
  }

  _xptr = _xs;
  _yptr = _ys;
}


/***************************************************************************************
** Function name:           pushColor
** Description:             Send a new pixel to the set window
***************************************************************************************/
void TFT_eSprite::pushColor(rgb_t color)
{
  Serial.printf("pushColor 1 %08x\n", color);

  if (!_created ) return;

  // Write the colour to RAM in set window
  if (_bpp == 16) {
    _img [_xptr + _yptr * _iwidth] = color24to16(color);
  }
  else  if (_bpp == 8) {
     uint8_t c8 = color24to8(color);
     Serial.printf("c8:%02x ", c8);
    _img8[_xptr + _yptr * _iwidth] = c8;
  }
  else if (_bpp == 4) {
    int idx = _cmap.getMapColorIdx(color);
    int index =  (_xptr + _yptr * _iwidth)>>1;
    if ((_xptr & 0x01) == 0) {
      _img4[index] = (idx << 4) | (_img4[index] & 0x0F);  // new color is in bits 7 .. 4
    }
    else {
      _img4[index] = (_img4[index] & 0xF0) | idx; // new color is the low bits
    }
  }

  else drawPixel(_clip, _xptr, _yptr, color);

  // Increment x
  _xptr++;

  // Wrap on x and y to start, increment y if needed
  if (_xptr > _xe) {
    _xptr = _xs;
    _yptr++;
    if (_yptr > _ye) _yptr = _ys;
  }

}


/***************************************************************************************
** Function name:           pushColor
** Description:             Send a "len" new pixels to the set window
***************************************************************************************/
void TFT_eSprite::pushColor(rgb_t pixelColor, uint32_t len)
{
  Serial.println("pushColor 2");

  if (!_created ) return;

  while(len--) writeColor(pixelColor);
}


/***************************************************************************************
** Function name:           writeColor
** Description:             Write a pixel with pre-formatted colour to the set window
***************************************************************************************/
void TFT_eSprite::writeColor(rgb_t color)
{
  Serial.printf(" %08x", color);

  if (!_created ) return;

  // Write 16-bit RGB 565 encoded colour to RAM
  if (_bpp == 16) {
    _img [_xptr + _yptr * _iwidth] = color24to16(color);
  }
  // Write 8-bit RGB 332 encoded colour to RAM
  else if (_bpp == 8) {
    uint8_t c8 = color24to8(color);
    Serial.printf("c8:%02x ", c8);
    _img8[_xptr + _yptr * _iwidth] = c8;
  }
  else if (_bpp == 4) {
    int idx = _cmap.getMapColorIdx(color);
    int index = (_xptr + _yptr * _iwidth)>>1;
    if ((_xptr & 0x01) == 0)
      _img4[index] = (idx << 4) | (_img4[index] & 0x0F);  // new color is in bits 7 .. 4
    else
      _img4[index] = (_img4[index] & 0xF0) | idx; // new color is the low bits (x is odd)
  }

  else drawPixel(_clip, _xptr, _yptr, color);

  // Increment x
  _xptr++;

  // Wrap on x and y to start, increment y if needed
  if (_xptr > _xe) {
    _xptr = _xs;
    _yptr++;
    if (_yptr > _ye) _yptr = _ys;
  }
}


/***************************************************************************************
** Function name:           setScrollRect
** Description:             Set scroll area within the sprite and the gap fill colour
***************************************************************************************/
// Intentionally not constrained to viewport area
void TFT_eSprite::setScrollRect(int32_t x, int32_t y, int32_t w, int32_t h, rgb_t color)
{
  Serial.println("setScrollRect");

  if ((x >= _iwidth) || (y >= _iheight) || !_created ) return;

  if (x < 0) { w += x; x = 0; }
  if (y < 0) { h += y; y = 0; }

  if ((x + w) > _iwidth ) w = _iwidth  - x;
  if ((y + h) > _iheight) h = _iheight - y;

  if ( w < 1 || h < 1) return;

  _sx = x;
  _sy = y;
  _sw = w;
  _sh = h;

  _scolor = color;
}


/***************************************************************************************
** Function name:           scroll
** Description:             Scroll dx,dy pixels, positive right,down, negative left,up
***************************************************************************************/
void TFT_eSprite::scroll(int16_t dx, int16_t dy)
{
  Serial.println("scroll");

  if (abs(dx) >= _sw || abs(dy) >= _sh) {
    fillRect (_clip, _sx, _sy, _sw, _sh, _scolor);
    return;
  }

  // Fetch the scroll area width and height set by setScrollRect()
  uint32_t w  = _sw - abs(dx); // line width to copy
  uint32_t h  = _sh - abs(dy); // lines to copy
  int32_t iw  = _iwidth;       // rounded up width of sprite

  // Fetch the x,y origin set by setScrollRect()
  uint32_t tx = _sx; // to x
  uint32_t fx = _sx; // from x
  uint32_t ty = _sy; // to y
  uint32_t fy = _sy; // from y

  // Adjust for x delta
  if (dx <= 0) fx -= dx;
  else tx += dx;

  // Adjust for y delta
  if (dy <= 0) fy -= dy;
  else { // Scrolling down so start copy from bottom
    ty = ty + _sh - 1; // "To" pointer
    iw = -iw;          // Pointer moves backwards
    fy = ty - dy;      // "From" pointer
  }

  // Calculate "from y" and "to y" pointers in RAM
  uint32_t fyp = fx + fy * _iwidth;
  uint32_t typ = tx + ty * _iwidth;

  // Now move the pixels in RAM
  if (_bpp == 16) {
    while (h--) { // move pixel lines (to, from, byte count)
      memmove( _img + typ, _img + fyp, w<<1);
      typ += iw;
      fyp += iw;
    }
  }
  else if (_bpp == 8) {
    while (h--) { // move pixel lines (to, from, byte count)
      memmove( _img8 + typ, _img8 + fyp, w);
      typ += iw;
      fyp += iw;
    }
  }
  else if (_bpp == 4) {
    // could optimize for scrolling by even # pixels using memove (later)
    if (dx >  0) { tx += w; fx += w; } // Start from right edge
    while (h--) { // move pixels one by one
      for (uint16_t xp = 0; xp < w; xp++) {
        if (dx <= 0) drawPixel(_clip, tx + xp, ty, readPixelValue(fx + xp, fy));
        if (dx >  0) drawPixel(_clip, tx - xp, ty, readPixelValue(fx - xp, fy));
      }
      if (dy <= 0)  { ty++; fy++; }
      else  { ty--; fy--; }
    }
  }
  else if (_bpp == 1 ) {
    if (dx >  0) { tx += w; fx += w; } // Start from right edge
    while (h--) { // move pixels one by one
      for (uint16_t xp = 0; xp < w; xp++) {
        if (dx <= 0) drawPixel(_clip, tx + xp, ty, readPixelValue(fx + xp, fy));
        if (dx >  0) drawPixel(_clip, tx - xp, ty, readPixelValue(fx - xp, fy));
      }
      if (dy <= 0)  { ty++; fy++; }
      else  { ty--; fy--; }
    }
  }
  else return; // Not 1, 4, 8 or 16 bpp

  // Fill the gap left by the scrolling
  if (dx > 0) fillRect(_clip, _sx, _sy, dx, _sh, _scolor);
  if (dx < 0) fillRect(_clip, _sx + _sw + dx, _sy, -dx, _sh, _scolor);
  if (dy > 0) fillRect(_clip, _sx, _sy, _sw, dy, _scolor);
  if (dy < 0) fillRect(_clip, _sx, _sy + _sh + dy, _sw, -dy, _scolor);
}


/***************************************************************************************
** Function name:           fillSprite
** Description:             Fill the whole sprite with defined colour
***************************************************************************************/
void TFT_eSprite::fillSprite(rgb_t color)
{
  Serial.println("fillSprite");

  if (!_created) return;

  Serial.printf("color = %08x   bpp = %d\n", color, _bpp);

  // Use memset if possible as it is super fast
  if(_clip.xDatum == 0 && _clip.yDatum == 0  &&  _xWidth == width()) {
    if(_bpp == 16) {
      fillRect(_clip, _clip.x1, _clip.y1, _xWidth, _yHeight, color);
    }
    else if (_bpp == 8) {
      uint8_t c8 = color24to8(color);
      Serial.printf("c8:%02x ", c8);
      memset(_img8, c8, _iwidth * _yHeight);
    }
    else if (_bpp == 4) {
      int idx = _cmap.getMapColorIdx(color);
      Serial.printf("idx = %02x\n", idx);
      uint8_t c2 = (idx & 0x0F) | ((idx & 0x0F) << 4);
      memset(_img4, idx, (_iwidth * _yHeight) >> 1);
    }
    else if (_bpp == 1) {
      int idx = _cmap.getMapColorIdx(color);
      Serial.printf("idx = %02x\n", idx);
      memset(_img8, idx == 1 ? 0xFF : 0x00, (_bitwidth>>3) * _dheight + 1);
    }
  }
  else {
    fillRect(_clip, _clip.x1 - _clip.xDatum, _clip.y1 - _clip.yDatum, _xWidth, _yHeight, color);
  }
}


/***************************************************************************************
** Function name:           width
** Description:             Return the width of sprite
***************************************************************************************/
// Return the size of the sprite
int16_t TFT_eSprite::width(void)
{
//  Serial.println("width");

  if (!_created ) return 0;

  if (_bpp > 1) {
    if (_vpDatum) return _xWidth;
    return _dwidth;
  }

  if (rotation & 1) {
    if (_vpDatum) return _xWidth;
    return _dheight;
  }

  if (_vpDatum) return _xWidth;
  return _dwidth;
}


/***************************************************************************************
** Function name:           height
** Description:             Return the height of sprite
***************************************************************************************/
int16_t TFT_eSprite::height(void)
{
//  Serial.println("height");

  if (!_created ) return 0;

  if (_bpp > 1) {
    if (_vpDatum) return _yHeight;
    return _dheight;
  }

  if (rotation & 1) {
    if (_vpDatum) return _yHeight;
    return _dwidth;
  }

  if (_vpDatum) return _yHeight;
  return _dheight;
}


/***************************************************************************************
** Function name:           setRotation
** Description:             Rotate coordinate frame for 1bpp sprite
***************************************************************************************/
// Does nothing for 4, 8 and 16 bpp sprites.
void TFT_eSprite::setRotation(uint8_t r)
{
  Serial.println("setRotation");

  if (_bpp != 1) return;

  rotation = r;
  
  if (rotation&1) {
    resetViewport();
  }
  else {
    resetViewport();
  }
}


/***************************************************************************************
** Function name:           getRotation
** Description:             Get rotation for 1bpp sprite
***************************************************************************************/
uint8_t TFT_eSprite::getRotation(void)
{
  return rotation;
}


/***************************************************************************************
** Function name:           drawPixel
** Description:             push a single pixel at an arbitrary position
***************************************************************************************/
void TFT_eSprite::drawPixel(clip_t& clip, int32_t x, int32_t y, rgb_t color)
{
  Serial.println("drawPixel");

  if (!_created) return;

  x+= clip.xDatum;
  y+= clip.yDatum;

  // Range checking
  if ((x < clip.x1) || (y < clip.y1) ||(x >= clip.x2) || (y >= clip.y2)) return;

  if (_bpp == 16) {
    _img[x+y*_iwidth] = color24to16(color);
  }
  else if (_bpp == 8) {
      uint8_t c8 = color24to8(color);
      Serial.printf("c8:%02 ", c8);
    _img8[x+y*_iwidth] = c8;
  }
  else if (_bpp == 4) {
    int idx = _cmap.getMapColorIdx(color);
    int index = (x + y*_iwidth)>>1;;
    if ((x & 0x01) == 0) {
      _img4[index] = ((idx & 0x0F) << 4) | (_img4[index] & 0x0F);
    }
    else {
      _img4[index] =  (idx & 0x0F) | (_img4[index] & 0xF0);
    }
  }
  else { // 1 bpp
    if (rotation == 1) {
      uint16_t tx = x;
      x = _dwidth - y - 1;
      y = tx;
    }
    else if (rotation == 2) {
      x = _dwidth - x - 1;
      y = _dheight - y - 1;
    }
    else if (rotation == 3) {
      uint16_t tx = x;
      x = y;
      y = _dheight - tx - 1;
    }

    if (color) _img8[(x + y * _bitwidth)>>3] |=  (0x80 >> (x & 0x7));
    else       _img8[(x + y * _bitwidth)>>3] &= ~(0x80 >> (x & 0x7));
  }
}


/***************************************************************************************
** Function name:           drawLine
** Description:             draw a line between 2 arbitrary points
***************************************************************************************/
void TFT_eSprite::drawLine(clip_t& clip, int32_t x0, int32_t y0, int32_t x1, int32_t y1, rgb_t color)
{
  Serial.println("drawLine");

  if (!_created) return;

  //_xDatum and _yDatum Not added here, it is added by drawPixel & drawFastxLine

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
        err += dx;
        if (dlen == 1) drawPixel(clip, y0, xs, color);
        else drawFastVLine(clip, y0, xs, dlen, color);
        dlen = 0; y0 += ystep; xs = x0 + 1;
      }
    }
    if (dlen) drawFastVLine(clip, y0, xs, dlen, color);
  }
  else {
    for (; x0 <= x1; x0++) {
      dlen++;
      err -= dy;
      if (err < 0) {
        err += dx;
        if (dlen == 1) drawPixel(clip, xs, y0, color);
        else drawFastHLine(clip, xs, y0, dlen, color);
        dlen = 0; y0 += ystep; xs = x0 + 1;
      }
    }
    if (dlen) drawFastHLine(clip, xs, y0, dlen, color);
  }
}


/***************************************************************************************
** Function name:           drawFastVLine
** Description:             draw a vertical line
***************************************************************************************/
void TFT_eSprite::drawFastVLine(clip_t& clip, int32_t x, int32_t y, int32_t h, rgb_t color)
{
  Serial.println("drawFastVLine");

  if (!_created) return;

  x+= clip.xDatum;
  y+= clip.yDatum;

  // Clipping
  if ((x < clip.x1) || (x >= clip.x2) || (y >= clip.y2)) return;

  if (y < clip.y1) { h += y - clip.y1; y = clip.y1; }

  if ((y + h) > clip.y2) h = clip.y2 - y;

  if (h < 1) return;

  if (_bpp == 16) {
    uint16_t c = color24to16(color);
    int32_t yp = x + _iwidth * y;
    while (h--) {_img[yp] = c; yp += _iwidth;}
  }
  else if (_bpp == 8) {
    uint8_t c8 = color24to8(color);
    Serial.printf("c8:%02x ", c8);
    while (h--) _img8[x + _iwidth * y++] = c8;
  }
  else if (_bpp == 4) {
    int idx = _cmap.getMapColorIdx(color);
    if ((x & 0x01) == 0) {
      while (h--) {
        int index = (x + _iwidth * y)>>1;
        _img4[index] = ((idx & 0x0F) << 4) | (_img4[index] & 0x0F);
        y++;
      }
    }
    else {
      while (h--) {
        int index = (x + _iwidth * y)>>1;
        _img4[index] = (idx & 0x0F) | (_img4[index] & 0xF0); // x is odd; new color goes into the low bits.
        y++;
      }
    }
  }
  else {
    x -= clip.xDatum; // Remove any offset as it will be added by drawPixel
    y -= clip.yDatum;
    while (h--) {
      drawPixel(clip, x, y++, color);
    }
  }
}


/***************************************************************************************
** Function name:           drawFastHLine
** Description:             draw a horizontal line
***************************************************************************************/
void TFT_eSprite::drawFastHLine(clip_t& clip, int32_t x, int32_t y, int32_t w, rgb_t color)
{
  Serial.println("drawFastHLine");

  if (!_created) return;

  x+= clip.xDatum;
  y+= clip.yDatum;

  // Clipping
  if ((y < clip.y1) || (x >= clip.x2) || (y >= clip.y2)) return;

  if (x < clip.x1) { w += x - clip.x1; x = clip.x1; }

  if ((x + w) > clip.x2) w = clip.x2 - x;

  if (w < 1) return;

  if (_bpp == 16) {
    uint16_t c = color24to16(color);
    while (w--) _img[_iwidth * y + x++] = c;
  }
  else if (_bpp == 8) {
    uint8_t c8 = color24to8(color);
    Serial.printf("c8:%02x ", c8);
    memset(_img8+_iwidth * y + x, c8, w);
  }
  else if (_bpp == 4) {
    if ((x & 0x01) == 1) {
      drawPixel(clip, x - clip.xDatum, y - clip.yDatum, color);
      x++; w--;
      if (w < 1)
        return;
    }

    if (((w + x) & 0x01) == 1) {
      // handle the extra one at the other end
      drawPixel(clip, x - clip.xDatum + w - 1, y - clip.yDatum, color);
      w--;
      if (w < 1) return;
    }
    int idx = _cmap.getMapColorIdx(color);
    uint8_t c2 = (idx & 0x0F) | ((idx & 0x0F) << 4);
    memset(_img4 + ((_iwidth * y + x) >> 1), c2, (w >> 1));
  }
  else {
    x -= clip.xDatum; // Remove any offset as it will be added by drawPixel
    y -= clip.yDatum;

    while (w--) {
      drawPixel(clip, x++, y, color);
    }
  }
}


/***************************************************************************************
** Function name:           fillRect
** Description:             draw a filled rectangle
***************************************************************************************/
void TFT_eSprite::fillRect(clip_t& clip, int32_t x, int32_t y, int32_t w, int32_t h, rgb_t color)
{
//  Serial.printf("fillRect %d,%d  %d*%d\n", x, y, w, h);

  if (!_created) return;

  x+= clip.xDatum;
  y+= clip.yDatum;

  // Clipping
  if ((x >= clip.x2) || (y >= clip.y2)) return;

  if (x < clip.x1) { w += x - clip.x1; x = clip.x1; }
  if (y < clip.y1) { h += y - clip.y1; y = clip.y1; }

  if ((x + w) > clip.x2) w = clip.x2 - x;
  if ((y + h) > clip.y2) h = clip.y2 - y;

  if ((w < 1) || (h < 1)) return;

  int32_t yp = _iwidth * y + x;

  if (_bpp == 16) {
    uint8_t c = color24to16(color);
    uint32_t iw = w;
    int32_t ys = yp;
    if(h--)  {while (iw--) _img[yp++] = c;}
    yp = ys;
    while (h--) {
      yp += _iwidth;
      memcpy( _img+yp, _img+ys, w<<1);
    }
  }
  else if (_bpp == 8) {
    uint8_t c8 = color24to8(color);
    Serial.printf("c8:%02x ", c8);
    while (h--) {
      memset(_img8 + yp, c8, w);
      yp += _iwidth;
    }
  }
  else if (_bpp == 4) {
    int idx = _cmap.getMapColorIdx(color);
    uint8_t c2 = (idx & 0x0F) | ((idx & 0x0F) << 4);
    if ((x & 0x01) == 0 && (w & 0x01) == 0) {
      yp = (yp >> 1);
      while (h--) {
        memset(_img4 + yp, c2, (w>>1));
        yp += (_iwidth >> 1);
      }
    }
    else if ((x & 0x01) == 0) {   // (w & 0x01) == 1

      // same as above but you have a hangover on the right.
      yp = (yp >> 1);
      while (h--) {
        if (w > 1)
          memset(_img4 + yp, c2, (w-1)>>1);
        // handle the rightmost pixel by calling drawPixel
        drawPixel(clip, x+w-1-clip.xDatum, y+h-clip.yDatum, color);
        yp += (_iwidth >> 1);
      }
    }
    else if ((w & 0x01) == 1) {   // (x & 0x01) == 0
      yp = (yp + 1) >> 1;
      while (h--) {
        drawPixel(clip, x-clip.xDatum, y+h-clip.yDatum, color);
        if (w > 1)
          memset(_img4 + yp, c2, (w-1)>>1);
        // same as above but you have a hangover on the left instead
        yp += (_iwidth >> 1);
      }
    }
    else {    // ((x & 0x01) == 1 && (w & 0x01) == 1)
      yp = (yp + 1) >> 1;
      while (h--) {
        drawPixel(clip, x-clip.xDatum, y+h-clip.yDatum, color);
        if (w > 1) drawPixel(clip, x+w-1-clip.xDatum, y+h-clip.yDatum, color);
        if (w > 2)
          memset(_img4 + yp, c2, (w-2)>>1);
        // maximal hacking, single pixels on left and right.
        yp += (_iwidth >> 1);
      }
    }
  }
  else {
    x -= clip.xDatum;
    y -= clip.yDatum;
    while (h--) {
      int32_t ww = w;
      int32_t xx = x;
      while (ww--) drawPixel(clip, xx++, y, color);
      y++;
    }
  }
}

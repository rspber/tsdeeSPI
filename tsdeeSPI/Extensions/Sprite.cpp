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
void TFT_eSprite::createPalette(const rgb_t colorMap[], uint8_t colors)
{
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
void TFT_eSprite::createPalette(const uint16_t colorMap[], uint8_t colors)
{
  if (!_created) return;

  if (colorMap == nullptr)
  {
    // Create a color map using the default FLASH map
    createPalette(default_4bit_palette, colors);
    return;
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
  if (_created) return _bpp;
  else return 0;
}


/***************************************************************************************
** Function name:           setBitmapColor
** Description:             Set the 1bpp foreground foreground and background colour
***************************************************************************************/
void TFT_eSprite::setBitmapColor(rgb_t c, rgb_t b)
{
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
  if (_cmap.cMap == nullptr || index >= _cmap.len) return; // out of bounds

  _cmap.cMap[index] = color;
}


/***************************************************************************************
** Function name:           getPaletteColor
** Description:             Return the palette color at 4bpp index, or 0 on error.
***************************************************************************************/
uint16_t TFT_eSprite::getPaletteColor(uint8_t index)
{
  if (_cmap.cMap == nullptr || index >= _cmap.len) return 0; // out of bounds

  return _cmap.cMap[index];
}


/***************************************************************************************
** Function name:           deleteSprite
** Description:             Delete the sprite to free up memory (RAM)
***************************************************************************************/
void TFT_eSprite::deleteSprite(void)
{
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
bool TFT_eSprite::pushRotated(int16_t angle, rgb_t transp_)
{
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

  uint16_t tpcolor = color24to16swap(transp_);

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
      uint16_t rp;
      if (_bpp == 16) {
        rp = _img[xp + yp * _iwidth];
      }
      else {
        rgb_t rgb = readPixel(_clip, xp, yp);
        rp = color24to16(rgb);
        rp = (uint16_t)(rp>>8 | rp<<8);
      }
      if (transp_ != 0x00FFFFFF && tpcolor == rp) {
        if (pixel_count) {
          // TFT window is already clipped, so this is faster than pushImage
          _tft->setWindow(x - pixel_count, y, pixel_count, 1);
          _tft->pushPixels16(sline_buffer, pixel_count);
          pixel_count = 0;
        }
      }
      else {
        sline_buffer[pixel_count++] = rp;
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
bool TFT_eSprite::pushRotated(TFT_eSprite *spr, int16_t angle, rgb_t transp_)
{
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
  uint16_t tpcolor = color24to16swap(transp_);

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
      uint16_t rp;
      if (_bpp == 16) {
        rp = _img[xp + yp * _iwidth];
      }
      else {
        rgb_t rgb = readPixel(_clip, xp, yp);
        spr->setSwapBytes(true);
        rp = color24to16(rgb);
      }
      if (transp_ != 0x00FFFFFF && tpcolor == rp) {
        if (pixel_count) {
          spr->pushImage16(x - pixel_count, y, pixel_count, 1, sline_buffer);
          pixel_count = 0;
        }
      }
      else {
        sline_buffer[pixel_count++] = rp;
      }
    } while (++x < max_x && (xs += _cosra) < xe && (ys += _sinra) < ye);
    if (pixel_count) {
      spr->pushImage16(x - pixel_count, y, pixel_count, 1, sline_buffer);
    }
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
  if (!_created) return;

  if (_bpp == 16) {
    bool oldSwapBytes = _tft->getSwapBytes();
    _tft->setSwapBytes(false);
    _tft->pushImage(x, y, _dwidth, _dheight, _img );
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
void TFT_eSprite::pushSprite(int32_t x, int32_t y, rgb_t transp_)
{
  if (!_created) return;

  if (_bpp == 16) {
    bool oldSwapBytes = _tft->getSwapBytes();
    _tft->setSwapBytes(false);
    _tft->pushImage16(x, y, _dwidth, _dheight, _img, transp_);
    _tft->setSwapBytes(oldSwapBytes);
  }
  else if (_bpp == 8) {
    cmap_t tmp = {0};
    _tft->pushImage16(x, y, _dwidth, _dheight, _img8, transp_, (bool)true, tmp);
  }
  else if (_bpp == 4) {
    _tft->pushImage16(x, y, _dwidth, _dheight, _img4, transp_, false, _cmap);
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

bool TFT_eSprite::pushToSprite(TFT_eSprite *dspr, int32_t x, int32_t y, rgb_t transp_)
{
  if ( !_created  || !dspr->_created) return false; // Check Sprites exist

  // Check destination sprite compatibility
  int8_t ds_bpp = dspr->getColorDepth();
  if (_bpp == 16 && ds_bpp != 16 && ds_bpp !=  8) return false;
  if (_bpp ==  8 && ds_bpp !=  8) return false;
  if (_bpp ==  4 || ds_bpp ==  4) return false;
  if (_bpp ==  1 && ds_bpp !=  1) return false;

  bool oldSwapBytes = dspr->getSwapBytes();
  uint16_t sline_buffer[width()];

  uint16_t transp = color24to16swap(transp_);

  // Scan destination bounding box and fetch transformed pixels from source Sprite
  for (int32_t ys = 0; ys < height(); ys++) {
    int32_t ox = x;
    uint32_t pixel_count = 0;

    for (int32_t xs = 0; xs < width(); xs++) {
      uint16_t rp = 0;
      if (_bpp == 16) {
        rp = _img[xs + ys * width()];
      }
      else {
        rgb_t rgb = readPixel(_clip, xs, ys);
        rp = color24to16swap(rgb);
      }
      //dspr->drawPixel(xs, ys, rp);

      if (transp == rp) {
        if (pixel_count) {
          dspr->pushImage16(ox, y, pixel_count, 1, sline_buffer);
          ox += pixel_count;
          pixel_count = 0;
        }
        ox++;
      }
      else {
        sline_buffer[pixel_count++] = rp;
      }
    }
    if (pixel_count) {
      dspr->pushImage16(ox, y, pixel_count, 1, sline_buffer);
    }
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
      while (sh--) {
        _tft->pushImage16(tx, ty++, sw, 1, _img + _xs + _iwidth * _ys++ );
      }
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
  uint8_t  blue[] = {0, 11, 21, 31};
};

/***************************************************************************************
** Function name:           readPixel
** Description:             Read 565 colour of a pixel at defined coordinates
***************************************************************************************/
rgb_t TFT_eSprite::readPixel(clip_t& clip, int32_t x, int32_t y)
{
  if (!_created) return 0xFFFF;

  x+= clip.xDatum;
  y+= clip.yDatum;

  // Range checking
  if ((x < clip.x1) || (y < clip.y1) ||(x >= clip.x2) || (y >= clip.y2)) return TFT_WHITE;

  if (_bpp == 16) {
    return color16to24swap(_img[x + y * _iwidth]);
  }

  if (_bpp == 8) {
    uint16_t c8 = _img8[x + y * _iwidth];
    if (c8 != 0) {
      uint16_t c16 =   (c8 & 0xE0)<<8 | (c8 & 0xC0)<<5
              | (c8 & 0x1C)<<6 | (c8 & 0x1C)<<3
              | blue[c8 & 0x03];
      return color16to24(c16);
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
    uint16_t lastColor = 0;
    uint8_t  color8    = 0;
    for (int32_t yp = dy; yp < dy + dh; yp++) {
      int32_t xyw = x + y * _iwidth;
      int32_t dxypw = dx + yp * w;
      for (int32_t xp = dx; xp < dx + dw; xp++) {
        uint16_t color = data[dxypw++];
        if (color != lastColor) {
          // When data source is a sprite, the bytes are already swapped
          if(!_swapBytes) color8 = (uint8_t)((color & 0xE0) | (color & 0x07)<<2 | (color & 0x1800)>>11);
          else color8 = (uint8_t)((color & 0xE000)>>8 | (color & 0x0700)>>6 | (color & 0x0018)>>3);
        }
        lastColor = color;
        _img8[xyw++] = color8;
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
          if ((xp & 0x01) == 0) {
            idx = (ptr[((xp+yp*w)>>1)] & 0xF0) >> 4; // even index = bits 7 .. 4
          }
          else {
            idx = ptr[((xp-1+yp*w)>>1)] & 0x0F;      // odd index = bits 3 .. 0.
          }
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
        uint16_t rgb16 = pgm_read_word(data + xp + yp * w);
        if(_swapBytes) rgb16 = rgb16<<8 | rgb16>>8;
        _img[ox + y * _iwidth] = rgb16;
        ox++;
      }
      y++;
    }
  }

  else if (_bpp == 8) { // Plot a 16 bpp image into a 8 bpp Sprite
    for (int32_t yp = dy; yp < dy + dh; yp++) {
      int32_t ox = x;
      for (int32_t xp = dx; xp < dx + dw; xp++) {
        uint16_t rgb16 = pgm_read_word(data + xp + yp * w);
        if(_swapBytes) rgb16 = rgb16<<8 | rgb16>>8;
        _img8[ox + y * _iwidth] = color16to8(rgb16);
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
  if (!_created ) return;

  // Write the colour to RAM in set window
  if (_bpp == 16) {
    _img [_xptr + _yptr * _iwidth] = color24to16swap(color);
  }
  else  if (_bpp == 8) {
     uint8_t c8 = color24to8(color);
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
  if (!_created ) return;

  while(len--) writeColor(pixelColor);
}


/***************************************************************************************
** Function name:           writeColor
** Description:             Write a pixel with pre-formatted colour to the set window
***************************************************************************************/
void TFT_eSprite::writeColor(rgb_t color)
{
  if (!_created ) return;

  // Write 16-bit RGB 565 encoded colour to RAM
  if (_bpp == 16) {
    _img [_xptr + _yptr * _iwidth] = color24to16swap(color);
  }
  // Write 8-bit RGB 332 encoded colour to RAM
  else if (_bpp == 8) {
    uint8_t c8 = color24to8(color);
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
  if (!_created) return;

  // Use memset if possible as it is super fast
  if(_clip.xDatum == 0 && _clip.yDatum == 0  &&  _xWidth == width()) {
    if(_bpp == 16) {
      fillRect(_clip, _clip.x1, _clip.y1, _xWidth, _yHeight, color);
    }
    else if (_bpp == 8) {
      uint8_t c8 = color24to8(color);
      memset(_img8, c8, _iwidth * _yHeight);
    }
    else if (_bpp == 4) {
      int idx = _cmap.getMapColorIdx(color);
      uint8_t c2 = (idx & 0x0F) | ((idx & 0x0F) << 4);
      memset(_img4, idx, (_iwidth * _yHeight) >> 1);
    }
    else if (_bpp == 1) {
      int idx = _cmap.getMapColorIdx(color);
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
  if (!_created) return;

  x+= clip.xDatum;
  y+= clip.yDatum;

  // Range checking
  if ((x < clip.x1) || (y < clip.y1) ||(x >= clip.x2) || (y >= clip.y2)) return;

  if (_bpp == 16) {
    _img[x+y*_iwidth] = color24to16swap(color);
  }
  else if (_bpp == 8) {
      uint8_t c8 = color24to8(color);
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
  if (!_created) return;

  x+= clip.xDatum;
  y+= clip.yDatum;

  // Clipping
  if ((x < clip.x1) || (x >= clip.x2) || (y >= clip.y2)) return;

  if (y < clip.y1) { h += y - clip.y1; y = clip.y1; }

  if ((y + h) > clip.y2) h = clip.y2 - y;

  if (h < 1) return;

  if (_bpp == 16) {
    uint16_t c = color24to16swap(color);
    int32_t yp = x + _iwidth * y;
    while (h--) {_img[yp] = c; yp += _iwidth;}
  }
  else if (_bpp == 8) {
    uint8_t c8 = color24to8(color);
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
    uint8_t c = color24to16swap(color);
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


#ifdef SMOOTH_FONT
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
#endif

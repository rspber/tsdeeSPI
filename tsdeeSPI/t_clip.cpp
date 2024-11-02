
#include "t_clip.h"

const int16_t ein_clip::width()
{
  return x2 > x1 ? x2 - x1 : 0;
}

const int16_t ein_clip::height()
{
  return y2 > y1 ? y2 - y1 : 0;
}

void ein_clip::set(int32_t x1, int32_t  y1, int32_t  x2, int32_t  y2)
{
  this->x1 = x1;
  this->y1 = y1;
  this->x2 = x2;
  this->y2 = y2;
}

bool ein_clip::check_point(int32_t& x, int32_t& y)
{
  x += xDatum;
  y += yDatum;

  if (x >= x2 || y >= y2) return false;
  if (x < x1 || y < y1) return false;

  return true;
}

bool ein_clip::check_block(block_t& z, const int32_t x0, const int32_t y0, const int32_t w, const int32_t h)
{
  z.x = x0 + xDatum;
  z.y = y0 + yDatum;

  if (z.x >= x2 || z.y >= y2) return false;

  z.dx = 0;
  z.dy = 0;
  z.dw = w;
  z.dh = h;

  if (z.x < x1) { z.dx = x1 - z.x; z.dw -= z.dx; z.x = x1; }
  if (z.y < y1) { z.dy = y1 - z.y; z.dh -= z.dy; z.y = y1; }

  if (z.x + z.dw > x2 ) z.dw = x2 - z.x;
  if (z.y + z.dh > y2 ) z.dh = y2 - z.y;

  if (z.dw < 1 || z.dh < 1) return false;

  return true;
}


/***************************************************************************************
** Function name:           clipWindow
** Description:             Clip window xs,yx,xe,ye to screen and viewport
***************************************************************************************/
bool ein_clip::clipClip(int32_t& xs, int32_t& ys, int32_t& xe, int32_t& ye)
{
  xs += xDatum;
  ys += yDatum;
  xe += xDatum;
  ye += yDatum;

  if ((xs >= x2) || (ys >= y2)) return false;
  if ((xe <  x1) || (ye <  y1)) return false;

  if (xs < x1) xs = x1;
  if (ys < y1) ys = y1;

  if (xe > x2) xe = x2 - 1;
  if (ye > y2) ye = y2 - 1;

  return true;
}


/***************************************************************************************
** Function name:           clipAddrWindow
** Description:             Clip address window x,y,w,h to screen and viewport
***************************************************************************************/
bool ein_clip::clipRect(int32_t& x, int32_t& y, int32_t& w, int32_t& h)
{
  x+= xDatum;
  y+= yDatum;

  if ((x >= x2) || (y >= y2)) return false;

  if (x < x1) { w += x - x1; x = x1; }
  if (y < y1) { h += y - y1; y = y1; }

  if ((x + w) > x2) w = x2 - x;
  if ((y + h) > y2) h = y2 - y;

  if ((w < 1) || (h < 1)) return false;

  return true;
}


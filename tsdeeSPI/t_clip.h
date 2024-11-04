
#pragma once

#include <stdint.h>
#include <stdbool.h>

struct co_block {
  int32_t x, y, dx, dy, dw, dh;
};

typedef co_block block_t;

struct ein_clip {
  int32_t  x1, y1, x2, y2;    // Note: x start, y start, x end + 1, y end + 1

  int32_t  xDatum;
  int32_t  yDatum;

  void set(int32_t  x1, int32_t  y1, int32_t  x2, int32_t  y2);

  bool check_point(int32_t& x, int32_t& y);

  bool clipRect(int32_t& x, int32_t& y, int32_t& w, int32_t& h);

  bool clipClip(int32_t& xs, int32_t& ys, int32_t& xe, int32_t& ye);

  void set(const ein_clip c);

  int32_t width();
  int32_t height();
};

typedef ein_clip clip_t;

struct wh_clip : public ein_clip {

  int32_t xWidth;
  int32_t yHeight;
};

typedef wh_clip wh_clip_t;

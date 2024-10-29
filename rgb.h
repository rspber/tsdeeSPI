
#pragma once

typedef uint32_t rgb_t;

inline rgb_t rgb16(const uint16_t color)
{
  return (rgb_t)(((uint32_t)(color & 0xF800) << 8) | ((color & 0x07E0) << 5) | (color & 0x1F) << 3);
}

inline rgb_t rgb24(const uint32_t color)
{
  return (rgb_t)color;
}

inline uint8_t rgb8(const uint32_t color) {
  return (color & 0xE00000)>>(5+8+6) | (color & 0xE000)>>(5 + 6) | (color & 0x00C0)>>6;
}

inline rgb_t unrgb8(const uint8_t color) {
  return (color & 0xE0)<<(5+8+6) | (color & 0x1C)>>(5 + 6) | (color & 0x03)<<6;
}

#ifdef COLOR_565
  typedef uint16_t mdt_t;
  #define MDT_SIZE 2
  #define rgb rgb16
#else
  typedef uint32_t mdt_t;
  #define MDT_SIZE 3
  #define rgb rgb24
#endif

#define TFT_WHITE RGB(0xFF, 0xFF, 0xFF)
#define TFT_BLACK RGB(0x00, 0x00, 0x00)

inline rgb_t RGB(uint8_t R, uint8_t G, uint8_t B) { return ((uint32_t)(R) << 16) | ((uint16_t)(G) << 8) | B; }

inline uint16_t RGB565(uint8_t R, uint8_t G, uint8_t B) { return ((uint16_t)(R >> 3) << 11) | ((uint16_t)(G >> 2) << 5) | (B >> 3); }

inline mdt_t mdt_color(const rgb_t color)
{
#ifdef COLOR_565
  return RGB565(color >> 16, color >> 8, color);
#else
  return color;
#endif
}

inline uint16_t swap16(const uint16_t v) { return (v >> 8) | (v << 8); }

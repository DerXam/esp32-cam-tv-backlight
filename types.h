#ifndef TG_CORE_TYPES_H
#define TG_CORE_TYPES_H

#include <Arduino.h>

struct Point {
  int x = 0, y = 0;
  Point operator+(const Point& o) const { return {x+o.x, y+o.y}; }
  Point operator-(const Point& o) const { return {x-o.x, y-o.y}; }
  Point operator*(float v) const { return { int(x*v), int(y*v) }; }
};

struct RGB { 
  uint8_t r=0,g=0,b=0; 

  // Saturating subtract: component-wise, clamped to [0,255]
  RGB operator-(const RGB& other) const {
    int rt = int(r) - int(other.r);
    int gt = int(g) - int(other.g);
    int bt = int(b) - int(other.b);
    return RGB{
      uint8_t(constrain(rt, 0, 255)),
      uint8_t(constrain(gt, 0, 255)),
      uint8_t(constrain(bt, 0, 255))
    };
  }

  // Scale by float with saturation
  RGB operator*(float f) const {
    int rt = int(r * f);
    int gt = int(g * f);
    int bt = int(b * f);
    return RGB{
      uint8_t(constrain(rt, 0, 255)),
      uint8_t(constrain(gt, 0, 255)),
      uint8_t(constrain(bt, 0, 255))
    };
  }
};

inline RGB operator*(float f, const RGB& c) { return c * f; }

inline RGB clampRGB(int r,int g,int b){
  return RGB{ uint8_t(constrain(r,0,255)),
              uint8_t(constrain(g,0,255)),
              uint8_t(constrain(b,0,255)) };
}

#endif // TG_CORE_TYPES_H

#ifndef TG_LEDS_LEDS_H
#define TG_LEDS_LEDS_H

#include <Arduino.h>
#include <Adafruit_NeoPixel.h>
#include "types.h"
#include "config.h"

// Forward declaration
struct AppState;

class Leds {
 public:
  void begin();
  void set(int idx, RGB target, const AppState& s);
  void show();
  inline int size() const { return TOTAL_LEDS; }
  const RGB* data() const { return buffer; }
  void writeJson(Print& out, const AppState& s) const;
private:
  Adafruit_NeoPixel strip{TOTAL_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800};
  RGB buffer[TOTAL_LEDS] = {};
};

#endif // TG_LEDS_LEDS_H

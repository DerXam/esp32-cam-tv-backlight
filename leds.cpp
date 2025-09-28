#include "leds.h"
#include "state.h"
#include "config.h"

void Leds::begin(){
  strip.begin();
  for (int i=0;i<TOTAL_LEDS;i++){ strip.setPixelColor(i, 0); buffer[i] = {0,0,0}; }
  strip.show();
}

int gamma8(int v, const AppState& s) {
  float f = v / 255.0f;
  int result = int(powf(f, s.ledGamma) * 255.0f + 0.5f);
  if (result < s.goBlackBelow) {
    result = 0;
  }
  return result;
}

void Leds::set(int i, RGB target, const AppState& s){
  if (i < 0 || i >= TOTAL_LEDS) return;
  RGB cur = buffer[i];
  float a = s.blendFactor; if (a < 0) a = 0; if (a > 1) a = 1;
  RGB out{
    gamma8(constrain(((cur.r + (int(target.r) - cur.r) * a) * s.ledFactor[0]),0,255),s),
    gamma8(constrain(((cur.g + (int(target.g) - cur.g) * a) * s.ledFactor[1]),0,255),s),
    gamma8(constrain(((cur.b + (int(target.b) - cur.b) * a) * s.ledFactor[2]),0,255),s)
  };
  buffer[i] = out;
  strip.setPixelColor(i, strip.Color(out.r, out.g, out.b));
}

void Leds::show(){ strip.show(); }

void Leds::writeJson(Print& out, const AppState& s) const {
  out.print(F("{\"h\":"));
  out.print(LEDS_HORIZONTAL);
  out.print(F(",\"v\":"));
  out.print(LEDS_VERTICAL);
  out.print(F(",\"first\":"));
  out.print(s.firstLed);
  out.print(F(",\"colors\":["));
  for (int i = 0; i < TOTAL_LEDS; ++i) {
    if (i) out.print(',');
    char buf[8];  // "#RRGGBB\0"
    snprintf(buf, sizeof(buf), "#%02X%02X%02X", buffer[i].r, buffer[i].g, buffer[i].b);
    out.print('\"');
    out.print(buf);
    out.print('\"');
  }
  out.print(F("]}"));
}
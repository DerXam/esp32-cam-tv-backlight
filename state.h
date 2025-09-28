#ifndef TG_STATE_STATE_H
#define TG_STATE_STATE_H

#include "types.h"

struct AppState {
  int ledDirection = 1;
  int firstLed = 0;
  int framesPerSecond = 5;
  float blendFactor = 0.5f;
  int   goBlackBelow = 50;

  int camBrightness = 255;
  int camGain = 0;
  int camContrast = 2;

  float vignetteStrength = 0.5f;
  float vignetteExponent = 2.0f;
  float hotspotX = 0.5f;
  float hotspotY = 0.5f;

  float colorMatrix[9] = {1,0,0, 0,1,0, 0,0,1};
  float pointFactor[7] = {1,1,1,1,1,1,1};
  float ledFactor[3] = {1,1,1};
  float ledGamma = 1.0f;
  float vert = 1.0f;

  Point points[7] = {};

  unsigned long testModeStart = 0;
  int nextPointNr = 0;
};

#endif // TG_STATE_STATE_H

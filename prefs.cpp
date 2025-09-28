#include "prefs.h"
#include <Preferences.h>

static Preferences prefs;

bool loadPrefs(AppState& s){
  prefs.begin("settings", true);
  int init = prefs.getInt("initialized", 0);
  if (init) {
    s.blendFactor = prefs.getFloat("blendFactor", s.blendFactor);
    s.vignetteStrength = prefs.getFloat("vignette", s.vignetteStrength);
    s.vignetteExponent = prefs.getFloat("vignetteExponent", s.vignetteExponent);
    s.hotspotX = prefs.getFloat("hotspotX", s.hotspotX);
    s.hotspotY = prefs.getFloat("hotspotY", s.hotspotY);
    s.goBlackBelow = prefs.getInt("black", s.goBlackBelow);
    s.framesPerSecond = prefs.getInt("fps", s.framesPerSecond);
    s.ledDirection = prefs.getInt("ledDir", s.ledDirection);
    s.firstLed = prefs.getInt("led1st", s.firstLed);
    s.camBrightness = prefs.getInt("brigth", s.camBrightness);
    s.camGain = prefs.getInt("gain", s.camGain);
    s.camContrast = prefs.getInt("contr", s.camContrast);
    prefs.getBytes("points", s.points, sizeof(s.points));
    prefs.getBytes("correction", s.colorMatrix, sizeof(s.colorMatrix));
    prefs.getBytes("pointFactor", s.pointFactor, sizeof(s.pointFactor));
    prefs.getBytes("ledFactor", s.ledFactor, sizeof(s.ledFactor));
    s.ledGamma = prefs.getFloat("ledGamma", s.ledGamma);
    s.vert = prefs.getFloat("vert", s.vert);
  }
  prefs.end();
  return true;
}

bool savePrefs(const AppState& s){
  prefs.begin("settings", false);
  prefs.clear();
  prefs.putInt("initialized", 1);
  prefs.putFloat("blendFactor", s.blendFactor);
  prefs.putFloat("vignette", s.vignetteStrength);
  prefs.putFloat("vignetteExponent", s.vignetteExponent);
  prefs.putFloat("hotspotX", s.hotspotX);
  prefs.putFloat("hotspotY", s.hotspotY);
  prefs.putInt("black", s.goBlackBelow);
  prefs.putInt("fps", s.framesPerSecond);
  prefs.putInt("ledDir", s.ledDirection);
  prefs.putInt("led1st", s.firstLed);
  prefs.putInt("brigth", s.camBrightness);
  prefs.putInt("gain", s.camGain);
  prefs.putInt("contr", s.camContrast);
  prefs.putBytes("points", s.points, sizeof(s.points));
  prefs.putBytes("correction", s.colorMatrix, sizeof(s.colorMatrix));
  prefs.putBytes("pointFactor", s.pointFactor, sizeof(s.pointFactor));
  prefs.putBytes("ledFactor", s.ledFactor, sizeof(s.ledFactor));
  prefs.putFloat("ledGamma", s.ledGamma);
  prefs.putFloat("vert", s.vert);
  prefs.end();
  return true;
}

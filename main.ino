#include <WiFi.h>
#include <ArduinoOTA.h>

#include "config.h"
#include "state.h"
#include "prefs.h"
#include "camera.h"
#include "leds.h"
#include "webserver.h"
#include "ota.h"

AppState S;
Leds LEDS;
WebServerLite WEB;

static inline int wrapIndex(int v, int mod){
  v %= mod;
  if (v < 0) v += mod;
  return v;
}

unsigned long nextTickMs = 0;
volatile bool otaActive = false;

void setup(){
  Serial.begin(115200);
  delay(100);

  Serial.print("WiFi setup ...");
  WiFi.begin(WIFI_SSID, WIFI_PSK);
  WiFi.setSleep(false);
  while (WiFi.status() != WL_CONNECTED) { delay(500); Serial.print("."); }
  Serial.println("DONE");
  Serial.println(WiFi.localIP());

  LEDS.begin();

  Serial.print("Cam setup ...");
  if (!cameraBegin()){
    Serial.println("❌ Couldn't initialise the camera");
  } else {
    Serial.println("DONE");
  }

  // Try to load persisted settings and apply to camera
  loadPrefs(S);
  applySensorFromState(S);

  WEB.begin(S);
  ArduinoOTA.setHostname(OTA_HOST);
  ArduinoOTA
    .onStart([](){ Serial.println("OTA Start"); otaActive = true;})
    .onEnd([](){ Serial.println("\nOTA End, reboot..."); otaActive = false;})
    .onProgress([](unsigned p, unsigned t){ Serial.printf("Progress: %u%%\r",(p*100)/t); })
    .onError([](ota_error_t e){ Serial.printf("Error[%u]\n", e); otaActive = false;});
  ArduinoOTA.begin();

}

static void updateEdges(camera_fb_t* fb){
  // Top and bottom. Be careful. We split them in half. We have 6 points, so 3 on top, 3 on bottom.
  for (int i = 0; i < LEDS_HORIZONTAL / 2; i++) {
    // Top left
    float percent = static_cast<float>(i) / (LEDS_HORIZONTAL / 2 - 1);
    Point p = S.points[1] + (S.points[2] - S.points[1]) * percent;
    float factor = S.pointFactor[1] + (S.pointFactor[2] - S.pointFactor[1]) * percent;
    RGB sample = samplePixel(fb, p, S) * factor;
    int pixelIndex = (S.firstLed + i * S.ledDirection) % TOTAL_LEDS;
    LEDS.set(pixelIndex, sample, S);
    
    // Top right
    p = S.points[2] + (S.points[3] - S.points[2]) * percent;
    factor = S.pointFactor[2] + (S.pointFactor[3] - S.pointFactor[2]) * percent;
    sample = samplePixel(fb, p, S) * factor;
    pixelIndex = (S.firstLed + LEDS_HORIZONTAL / 2 + i * S.ledDirection) % TOTAL_LEDS;
    LEDS.set(pixelIndex, sample, S);

    // Bottom right
    p = S.points[5] + (S.points[4] - S.points[5]) * percent;
    factor = S.pointFactor[5] + (S.pointFactor[4] - S.pointFactor[5]) * percent;
    sample = samplePixel(fb, p, S) * factor;
    pixelIndex = (S.firstLed + LEDS_HORIZONTAL + LEDS_VERTICAL + i * S.ledDirection) % TOTAL_LEDS;
    LEDS.set(pixelIndex, sample, S);

    // Bottom left
    p = S.points[6] + (S.points[5] - S.points[6]) * percent;
    factor = S.pointFactor[6] + (S.pointFactor[5] - S.pointFactor[6]) * percent;
    sample = samplePixel(fb, p, S) * factor;
    pixelIndex = (S.firstLed + LEDS_HORIZONTAL / 2 + LEDS_HORIZONTAL + LEDS_VERTICAL + i * S.ledDirection) % TOTAL_LEDS;
    LEDS.set(pixelIndex, sample, S);
    
  }
  
  // Left and right
  for (int i = 0; i < LEDS_VERTICAL; i++) {
    float orgPercent = static_cast<float>(i) / (LEDS_VERTICAL - 1);
    // Factor in the camera distortion. This is quite strong if the camera is 200° and sits right on top of the screen.
    float percent = powf(orgPercent, S.vert);
    Point p = S.points[3] + (S.points[4] - S.points[3]) * percent;
    float factor = S.pointFactor[3] + (S.pointFactor[4] - S.pointFactor[3]) * percent;
    RGB sample = samplePixel(fb, p, S) * factor;
    int pixelIndex = (S.firstLed + LEDS_HORIZONTAL + i * S.ledDirection) % TOTAL_LEDS;
    LEDS.set(pixelIndex, sample, S);

    percent = powf(orgPercent, 1/S.vert);
    p = S.points[6] + (S.points[1] - S.points[6]) * (percent);
    factor = S.pointFactor[6] + (S.pointFactor[1] - S.pointFactor[6]) * (percent);
    sample = samplePixel(fb, p, S) * factor;
    pixelIndex = (S.firstLed + LEDS_HORIZONTAL * 2 + LEDS_VERTICAL + i * S.ledDirection) % TOTAL_LEDS;
    LEDS.set(pixelIndex, sample, S);
  }

  LEDS.show();
}

static void runTestMode(){
  // This will show the borders in distinctive colors, so you can do some debugging on the amount of LEDs on each side
  // Will stop after 10s. 
  unsigned long sec = millis() - S.testModeStart;
  if (sec > 10000) { S.testModeStart = 0; return; }

  int whitePixel = sec / 200;

  for (int i = 0; i < LEDS_HORIZONTAL; i++) {
    int idxTop = wrapIndex(S.firstLed + i * S.ledDirection, TOTAL_LEDS);
    LEDS.set(idxTop, (i == whitePixel % LEDS_HORIZONTAL) ? RGB{255,255,255} : RGB{255,0,0}, S);

    int idxBot = wrapIndex(S.firstLed + LEDS_HORIZONTAL + LEDS_VERTICAL + i * S.ledDirection, TOTAL_LEDS);
    LEDS.set(idxBot, (i == whitePixel % LEDS_HORIZONTAL) ? RGB{255,255,255} : RGB{0,255,0}, S);
  }
  for (int i = 0; i < LEDS_VERTICAL; i++) {
    int idxR = wrapIndex(S.firstLed + LEDS_HORIZONTAL + i * S.ledDirection, TOTAL_LEDS);
    LEDS.set(idxR, ((whitePixel/2) % LEDS_VERTICAL == i) ? RGB{255,255,255} : RGB{0,0,255}, S);

    int idxL = wrapIndex(S.firstLed + LEDS_HORIZONTAL*2 + LEDS_VERTICAL + i * S.ledDirection, TOTAL_LEDS);
    LEDS.set(idxL, ((whitePixel/2) % LEDS_VERTICAL == i) ? RGB{255,255,255} : RGB{0,0,255}, S);
  }

  LEDS.show();
}

void loop(){
  ArduinoOTA.handle();
  if (otaActive || Update.isRunning()) {
    yield();
    delay(0);
    return;
  }
  if (S.testModeStart){
    runTestMode();
    WEB.handle(S, LEDS);
    return;
  }

  if (millis() >= nextTickMs){
    nextTickMs = millis() + (1000UL / (uint16_t)max(1, S.framesPerSecond));
    camera_fb_t* fb = cameraGrab();
    if (fb && fb->format == PIXFORMAT_YUV422){
      // Update edges
      updateEdges(fb);

      cameraRelease(fb);
    } else {
      if (fb) cameraRelease(fb);
      Serial.print("CamErr...");
    }
  }

  WEB.handle(S, LEDS);
}

#include "webserver.h"
#include "index_html.h"
#include "config.h"
#include "state.h"
#include "prefs.h"

void WebServerLite::begin(const AppState&){
  server.begin();
}

void WebServerLite::sendTextHeader(WiFiClient& c){
  c.println("HTTP/1.1 200 OK");
  c.println("Content-Type: text/html; charset=utf-8");
  c.println("Connection: close");
  c.println();
}
void WebServerLite::sendJsonHeader(WiFiClient& c){
  c.println(F("HTTP/1.1 200 OK"));
  c.println(F("Content-Type: application/json; charset=utf-8"));
  c.println(F("Cache-Control: no-cache"));
  c.println(F("Connection: close"));
  c.println();
}
void WebServerLite::sendBmpHeader(WiFiClient& c){
  c.println("HTTP/1.1 200 OK");
  c.println("Content-Type: image/bmp");
  c.println("Connection: close");
  c.println();
}

void WebServerLite::handle(AppState& s, Leds& leds){
  WiFiClient client = server.available();
  if (!client) return;

  String reqLine = client.readStringUntil('\r'); client.readStringUntil('\n');
  String method = reqLine.substring(0, reqLine.indexOf(' '));
  String path   = reqLine.substring(reqLine.indexOf(' ')+1, reqLine.lastIndexOf(' '));

  if (method == "GET" && path == "/") {
    handleRoot(client); client.stop(); return;
  }
  if (method == "GET" && path.startsWith("/image.bmp")) {
    handleImage(client, s); client.stop(); return;
  }
  if (method == "GET" && path.startsWith("/testMode")) {
    s.testModeStart = millis();
    sendTextHeader(client);
    client.print("OK");
    client.stop(); return;
  }
  if (method == "GET" && path.startsWith("/settings")) {
    handleSettingsGet(client, s); client.stop(); return;
  }
  if (method == "GET" && path.startsWith("/input")) {
    // /input?key=value
    char param[32] = {0}, value[64] = {0};
    if (sscanf(path.c_str(), "/input?%31[^=]=%63[^&]", param, value) == 2) {
      handleInputSet(client, s, param, value);
    } else {
      sendTextHeader(client);
    }
    client.stop(); return;
  }

  if (method == "GET" && path.startsWith("/click")) {
    int x = 0, y = 0;
    if (sscanf(path.c_str(), "/click?x=%d&y=%d", &x, &y) == 2) {
      handleClick(client, s, x, y);
    } else {
      client.println("HTTP/1.1 204 No Content"); client.println("Connection: close"); client.println();
    }
    client.stop(); return;
  }

  if (method == "GET" && path.startsWith("/leds")) {
    handleLedsJson(client, leds, s);
    return;
  }

  // default
  client.println("HTTP/1.1 404 Not Found"); client.println("Connection: close"); client.println();
  client.stop();
}

void WebServerLite::handleRoot(WiFiClient& c){
  sendTextHeader(c);
  c.print((__FlashStringHelper*)INDEX_HTML);
}

void WebServerLite::handleSettingsGet(WiFiClient& c, const AppState& s){
  sendTextHeader(c);
  c.print("{\"blendFactor\": " + String(s.blendFactor, 2));
  c.print(",\"vignette\": " + String(s.vignetteStrength, 2));
  c.print(",\"vignetteExponent\": " + String(s.vignetteExponent, 2));
  c.print(",\"hotspotX\": " + String(s.hotspotX, 2));
  c.print(",\"hotspotY\": " + String(s.hotspotY, 2));
  c.printf(",\"black\": %d", s.goBlackBelow);
  c.printf(",\"fps\": %d", s.framesPerSecond);
  c.printf(",\"ledDir\": %d", s.ledDirection);
  c.printf(",\"led1st\": %d", s.firstLed);
  c.printf(",\"brigth\": %d", s.camBrightness);
  c.printf(",\"gain\": %d", s.camGain);
  c.printf(",\"contr\": %d", s.camContrast);
  c.print(",\"ledGamma\": " + String(s.ledGamma, 2));
  c.print(",\"vert\": " + String(s.vert, 2));
  for (int i=0;i<9;i++) c.print(String(",\"correction")+i+"\": " + String(s.colorMatrix[i], 2));
  for (int i=1;i<=6;i++) c.print(String(",\"pointFactor")+i+"\": " + String(s.pointFactor[i], 2));
  for (int i=0;i<3;i++) c.print(String(",\"ledFactor")+i+"\": " + String(s.ledFactor[i], 2));
  for (int i=1;i<=6;i++) c.print(String(",\"points")+i+"\": {\"x\": " + s.points[i].x+ ", \"y\": " + s.points[i].y+"}");
  c.print("}");
}

void WebServerLite::handleInputSet(WiFiClient& c, AppState& s, const char* k, const char* v){
  String key(k);
  String val(v);

  if (key == "blendFactor") s.blendFactor = atof(v);
  else if (key == "vignette") s.vignetteStrength = atof(v);
  else if (key == "vignetteExponent") s.vignetteExponent = atof(v);
  else if (key == "black") s.goBlackBelow = atoi(v);
  else if (key == "fps") s.framesPerSecond = atoi(v);
  else if (key == "ledDir") s.ledDirection = atoi(v);
  else if (key == "led1st") s.firstLed = atoi(v);
  else if (key == "brigth") s.camBrightness = atoi(v);
  else if (key == "gain") s.camGain = atoi(v);
  else if (key == "contr") s.camContrast = atoi(v);
  else if (key == "hotspotX") s.hotspotX = atof(v);
  else if (key == "hotspotY") s.hotspotY = atof(v);
  else if (key == "ledGamma") s.ledGamma = atof(v);
  else if (key == "vert") s.vert = atof(v);
  else if (key.startsWith("correction")){
    int idx = key.substring(10).toInt();
    if (idx >= 0 && idx < 9) s.colorMatrix[idx] = atof(v);
  } else if (key.startsWith("pointFactor")){
    int idx = key.substring(11).toInt();
    if (idx >= 1 && idx <= 6) s.pointFactor[idx] = atof(v);
  } else if (key.startsWith("ledFactor")){
    int idx = key.substring(9).toInt();
    if (idx >= 0 && idx <= 2) s.ledFactor[idx] = atof(v);
  }

  savePrefs(s);
  applySensorFromState(s);
  sendTextHeader(c);
  c.print("OK");
}

void WebServerLite::handleClick(WiFiClient& c, AppState& s, int x, int y){
  int idx = (s.nextPointNr % 6) + 1;
  s.points[idx] = { x, y };
  sendTextHeader(c);
  c.printf("Registered click %d at (%d, %d)<br/>", idx, x, y);
  s.nextPointNr++;
}

void WebServerLite::handleImage(WiFiClient& c, const AppState& s){
  // First get and discard one frame (quirk), then grab fresh one
  camera_fb_t* oldFb = cameraGrab();
  if (oldFb) cameraRelease(oldFb);
  camera_fb_t* fb = cameraGrab();
  if (!fb || fb->format != PIXFORMAT_YUV422) {
    if (fb) cameraRelease(fb);
    c.println("HTTP/1.1 500 Internal Server Error"); c.println("Connection: close"); c.println();
    return;
  }

  const int width = fb->width;
  const int height = fb->height;
  const int headersize24 = 54;
  const int rowSize = width * 3;
  const int padding = (4 - (rowSize % 4)) % 4;
  const size_t bufSize = rowSize + padding;

  // BMP header
  sendBmpHeader(c);
  uint8_t h24[54] = {0};
  h24[0] = 'B'; h24[1] = 'M';
  int32_t dataSize = (rowSize + padding) * height;
  int32_t fileSize = headersize24 + dataSize;
  h24[2] = fileSize & 0xFF; h24[3] = (fileSize >> 8) & 0xFF; h24[4] = (fileSize >> 16) & 0xFF; h24[5] = (fileSize >> 24) & 0xFF;
  h24[10] = headersize24; // data offset
  h24[14] = 40; // DIB
  h24[18] = width & 0xFF; h24[19] = (width >> 8) & 0xFF; h24[20] = (width >> 16) & 0xFF; h24[21] = (width >> 24) & 0xFF;
  int32_t negH = -height;
  h24[22] = negH & 0xFF; h24[23] = (negH >> 8) & 0xFF; h24[24] = (negH >> 16) & 0xFF; h24[25] = (negH >> 24) & 0xFF;
  h24[26] = 1; h24[27] = 0; // planes
  h24[28] = 24; h24[29] = 0; // 24-bit
  c.write(h24, 54);

  // Stream rows
  uint8_t* lineBuf = (uint8_t*)malloc(bufSize);
  if (!lineBuf){ cameraRelease(fb); return; }

  for (int y=0; y<height; y++){
    uint8_t* p = lineBuf;  // BGR...
    for (int x=0; x<width; x++){
      RGB col = samplePixel(fb, Point{x,y}, s);
      *p++ = col.b; *p++ = col.g; *p++ = col.r;
    }
    for (int i=0;i<padding;i++) *p++ = 0;
    c.write(lineBuf, bufSize);
  }
  free(lineBuf);

  cameraRelease(fb);
}

void WebServerLite::handleLedsJson(WiFiClient& c, const Leds& leds, const AppState& s) {
  sendJsonHeader(c);
  leds.writeJson(c, s);
  c.stop();
}

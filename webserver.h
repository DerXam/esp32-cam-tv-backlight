#ifndef TG_WEB_SERVER_H
#define TG_WEB_SERVER_H

#include <WiFi.h>
#include "camera.h"
#include "leds.h"

// Forward declaration
struct AppState;

class WebServerLite {
 public:
  void begin(const AppState& s);
  void handle(AppState& s, Leds& leds);
 private:
  WiFiServer server{80};
  void sendTextHeader(WiFiClient& c);
  void sendJsonHeader(WiFiClient& c);
  void sendBmpHeader(WiFiClient& c);
  void handleRoot(WiFiClient& c);
  void handleImage(WiFiClient& c, const AppState& s);
  void handlePoll(WiFiClient& c, const AppState& s);
  void handleSettingsGet(WiFiClient& c, const AppState& s);
  void handleInputSet(WiFiClient& c, AppState& s, const char* k, const char* v);
  void handleClick(WiFiClient& c, AppState& s, int x, int y);
  void handleLedsJson(WiFiClient& c, const Leds& leds, const AppState& s);
};

#endif // TG_WEB_SERVER_H

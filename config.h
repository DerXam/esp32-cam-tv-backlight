#ifndef TG_CORE_BUILD_H
#define TG_CORE_BUILD_H

#include <Arduino.h>

constexpr int LEDS_HORIZONTAL = 40;
constexpr int LEDS_VERTICAL   = 22;
constexpr int TOTAL_LEDS      = LEDS_HORIZONTAL*2 + LEDS_VERTICAL*2;

inline constexpr const char* WIFI_SSID = "PUT_YOUR_WIFI_IN_HERE";
inline constexpr const char* WIFI_PSK  = "AND_YOUR_PASSWORD";
inline constexpr const char* OTA_HOST  = "esp32-ambilight";

// Camera pins (ESP32-CAM)
#define PWDN_GPIO_NUM 32
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 0
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 21
#define Y4_GPIO_NUM 19
#define Y3_GPIO_NUM 18
#define Y2_GPIO_NUM 5
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

// LED strip
#define LED_PIN 2

#endif
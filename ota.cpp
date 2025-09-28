#include "ota.h"
#include <ArduinoOTA.h>
#include <Arduino.h>

void otaBegin(const char* hostname){
  ArduinoOTA.setHostname(hostname);
  ArduinoOTA
    .onStart([](){ Serial.println("OTA Start"); })
    .onEnd([](){ Serial.println("\nOTA End, reboot..."); })
    .onProgress([](unsigned p, unsigned t){ Serial.printf("Progress: %u%%\r",(p*100)/t); })
    .onError([](ota_error_t e){ Serial.printf("Error[%u]\n", e); });
  ArduinoOTA.begin();
}

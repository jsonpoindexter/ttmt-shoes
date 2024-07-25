#ifndef OTA_HANDLER_H
#define OTA_HANDLER_H

#include <ArduinoOTA.h>

void initOTA(const char *ssid, const char *password, const char *hostname);

void handleOTA();

#endif // OTA_HANDLER_H
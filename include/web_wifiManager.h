#ifndef WEB_WIFIMANAGER
#define WEB_WIFIMANAGER

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "LittleFS.h"

#include "constants.h"
#include "web_cards.h"

extern String ssid, pass, ip, gateway;

void beginWifiManager();
bool initWiFi();
void setupSSID();
void loopWifiManager();

#endif
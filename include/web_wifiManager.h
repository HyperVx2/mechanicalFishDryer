#ifndef WEB_WIFIMANAGER
#define WEB_WIFIMANAGER

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "LittleFS.h"

#include "constants.h"
#include "web_cards.h"

extern String ssid;
extern String ip;

void beginWifiManager();
void setupSSID();
void loopWifiManager();

#endif
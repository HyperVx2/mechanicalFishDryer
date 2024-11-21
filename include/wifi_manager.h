#ifndef WIFI_MANAGER
#define WIFI_MANAGER

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "LittleFS.h"

#include "constants.h"
#include "web_serve.h"
#include "web_cards.h"

extern String ssid;
extern String ip;

void initLittleFS();
String readFile(fs::FS &fs, const char * path);
void writeFile(fs::FS &fs, const char * path, const char * message);
bool initWiFi();

void setupWebserver();

void web_sendEvent();

#endif
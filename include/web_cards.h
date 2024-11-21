#ifndef WEB_CARDS
#define WEB_CARDS

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include <Arduino_JSON.h>

#include "constants.h"
#include "sensors.h"
#include "display_oled.h"
#include "web_processor.h"

extern float humidity, temperature, weight;
String getSensorReadings();

void home(AsyncWebServer *server);
void card_0(AsyncWebServer *server);
void card_1(AsyncWebServer *server);
void card_2(AsyncWebServer *server);
void card_3(AsyncWebServer *server);

#endif

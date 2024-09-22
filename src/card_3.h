#ifndef CARD_3
#define CARD_3

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "processor.h"
#include <Arduino_JSON.h>

extern float humidity, temperature, weight;
String getSensorReadings();

void card_3(AsyncWebServer *server);
#endif
#ifndef WEB_CARDS
#define WEB_CARDS

#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <AsyncTCP.h>
#include "LittleFS.h"
#include <Arduino_JSON.h>

#include "constants.h"
#include "gpio_actuators.h"
#include "gpio_sensors.h"
#include "util_timer.h"

void serve(AsyncWebServer *server);

String processor(const String& var);
String getSensorReadings();

#endif
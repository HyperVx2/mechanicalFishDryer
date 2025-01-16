#ifndef WEB_CARDS
#define WEB_CARDS

#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>
#include <PubSubClient.h>
#include <AsyncTCP.h>

#include "constants.h"
#include "web_wifiManager.h"
#include "gpio_actuators.h"
#include "gpio_sensors.h"
#include "util_timer.h"

String getReadings();

void beginMQTT();
void loopMQTT();

void serve(AsyncWebServer *server);

#endif
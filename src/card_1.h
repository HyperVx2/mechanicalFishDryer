/* card_0: function to trigger RELAY HEATER off/on via a /off or /on url
*  "/on1" - url to turn on
*  "/off1" - url to turn off
*/
#ifndef CARD_1
#define CARD_1

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "processor.h"

// ON1/OFF1 Properties
#define relay_heat 33

void card_1(AsyncWebServer *server);
#endif

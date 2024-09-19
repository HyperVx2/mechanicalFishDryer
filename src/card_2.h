/* card_0: function to trigger RELAY FAN off/on via a /off or /on url
*  "/on1" - url to turn on
*  "/off1" - url to turn off
*/
#ifndef CARD_2
#define CARD_2

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "processor.h"

// ON1/OFF1 Properties
#define relay_fan 14

void card_2(AsyncWebServer *server);
#endif

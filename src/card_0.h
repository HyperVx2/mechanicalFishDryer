/* card_0: a function to set the dryer status using a /off or /on url
*  "/on0" - url to turn on
*  "/off0" - url to turn off
*/

#ifndef CARD_0
#define CARD_0

#include <ESPAsyncWebServer.h>
#include <LittleFS.h>
#include "processor.h"

extern bool dryerStatus;

void card_0(AsyncWebServer *server);
#endif

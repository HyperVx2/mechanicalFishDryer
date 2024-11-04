/* serve - very simple function which provides a central location 
*  serving cards. For every card on the home page, there needs to 
*  a call to the card in serve. Think of this as the "void loop" in 
*  the Arduino .ino file for the website.
*/

#ifndef WEB_SERVE
#define WEB_SERVE

#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <LittleFS.h>

#include "web_cards.h"

void serve(AsyncWebServer *server);
#endif
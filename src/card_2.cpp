#include "card_2.h"

extern AsyncWebServer server();

void card_2(AsyncWebServer *server) {

    // Route to set relay fan to LOW
    server->on("/on1", HTTP_GET, [](AsyncWebServerRequest *request){
        digitalWrite(relay_fan, LOW);
        request->send(LittleFS, "/index.html", String(), false, processor);
    });

    // Route to set relay fan to HIGH
    server->on("/off1", HTTP_GET, [](AsyncWebServerRequest *request){
        digitalWrite(relay_fan, HIGH);
        request->send(LittleFS, "/index.html", String(), false, processor);
    });
}
#include "card_1.h"

extern AsyncWebServer server();

void card_1(AsyncWebServer *server) {

    // Route to set relay heater to LOW
    server->on("/on1", HTTP_GET, [](AsyncWebServerRequest *request){
        digitalWrite(relay_heat, LOW);
        request->send(LittleFS, "/index.html", String(), false, processor);
    });

    // Route to set relay heater to HIGH
    server->on("/off1", HTTP_GET, [](AsyncWebServerRequest *request){
        digitalWrite(relay_heat, HIGH);
        request->send(LittleFS, "/index.html", String(), false, processor);
    });
}
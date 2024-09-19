#include "card_0.h"

extern AsyncWebServer server();

void card_0(AsyncWebServer *server) {

    // Route to set dryer status to TRUE
    server->on("/on0", HTTP_GET, [](AsyncWebServerRequest *request){
        dryerStatus = true; 
        request->send(LittleFS, "/index.html", String(), false, processor);
    });

    // Route to set dryer status to FALSE
    server->on("/off0", HTTP_GET, [](AsyncWebServerRequest *request){
        dryerStatus = false;    
        request->send(LittleFS, "/index.html", String(), false, processor);
    });

}
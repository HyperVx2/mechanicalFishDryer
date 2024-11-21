#include "web_cards.h"

extern AsyncWebServer server();

// Json Variable to Hold Sensor Readings
JSONVar readings;

void home(AsyncWebServer *server) {
    // Route for root / web page

    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
    });

    server->serveStatic("/", LittleFS, "/");

    server->onNotFound([](AsyncWebServerRequest *request){
        request->send(404, "text/html", "<h1>The file or content was not found.</h1>");
    });

}

void card_0(AsyncWebServer *server) {

    // Route to set dryer status to TRUE
    server->on("/on0", HTTP_GET, [](AsyncWebServerRequest *request){
        setTareHX711();
        display_addNotification("Tare set");
        request->send(LittleFS, "/index.html", String(), false, processor);
    });

    // Route to set dryer status to FALSE
    server->on("/off0", HTTP_GET, [](AsyncWebServerRequest *request){
        setTareHX711();
        display_addNotification("Tare set");
        request->send(LittleFS, "/index.html", String(), false, processor);
    });

}

void card_1(AsyncWebServer *server) {

    // Route to set relay heater to LOW
    server->on("/on1", HTTP_GET, [](AsyncWebServerRequest *request){
        digitalWrite(RELAY_HEAT, LOW);
        display_addNotification("Heater ON");
        request->send(LittleFS, "/index.html", String(), false, processor);
    });

    // Route to set relay heater to HIGH
    server->on("/off1", HTTP_GET, [](AsyncWebServerRequest *request){
        digitalWrite(RELAY_HEAT, HIGH);
        display_addNotification("Heater OFF");
        request->send(LittleFS, "/index.html", String(), false, processor);
    });
}

void card_2(AsyncWebServer *server) {

    // Route to set relay fan to LOW
    server->on("/on2", HTTP_GET, [](AsyncWebServerRequest *request){
        digitalWrite(RELAY_FAN, LOW);
        display_addNotification("Fan ON");
        request->send(LittleFS, "/index.html", String(), false, processor);
    });

    // Route to set relay fan to HIGH
    server->on("/off2", HTTP_GET, [](AsyncWebServerRequest *request){
        digitalWrite(RELAY_FAN, HIGH);
        display_addNotification("Fan OFF");
        request->send(LittleFS, "/index.html", String(), false, processor);
    });
}

// Get Sensor Readings and return JSON object
String getSensorReadings(){
    readings["millis"] = String(millis());
    readings["temperature"] = String(temperature);
    readings["humidity"] =  String(humidity);
    readings["weight"] = String(weight);
    
    String jsonString = JSON.stringify(readings);
    return jsonString;
}

void card_3(AsyncWebServer *server) {

    // Send a GET request to /slider?value=<inputMessage>
    // Request for the latest sensor readings
    server->on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
        String json = getSensorReadings();
        request->send(200, "application/json", json);
        json = String();
    });
}
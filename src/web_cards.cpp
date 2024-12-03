#include "web_cards.h"

extern AsyncWebServer server();

// Json varaible to hold sensor readings
JSONVar readings;

// Note that server is a pointer entering serve(),
// it doesn't need to be derefenced again.
void serve(AsyncWebServer *server) {
    // Home Page
    home(server);

    // Card 0: Set tare
    // Route to set dryer status
    card_0(server);

    // Card 1: Relay heater ON/OFF
    // Route to relay pin ON or OFF
    card_1(server);

    // Card 2: Relay fan ON/OFF
    // Route to relay pin ON or OFF
    card_2(server);

    // Card 3: Sensor readings
    // This will read three sensor readings
    card_3(server);
}

// Get actuators state
String processor(const String& var) {
    String relayHeatState;
    String relayFanState;

    //debug* Serial.println(var);

    if (var == "RELAYHEAT") {
        if (digitalRead(RELAY_HEAT)){
            relayHeatState = "OFF";
        }
        else {
            relayHeatState = "ON";
        }
        Serial.print(relayHeatState);
        return relayHeatState;
    }

    // Card 2 Processing
    if (var == "RELAYFAN") {
        if (digitalRead(RELAY_FAN)){
            relayFanState = "OFF";
        }
        else {
            relayFanState = "ON";
        }
        Serial.print(relayFanState);
        return relayFanState;
    }
}

// Get sensor readings and return JSON object
String getSensorReadings() {
    readings["timeRemaining"] = getRemainingTime();
    readings["temperature"] = temperature;
    readings["humidity"] = humidity;
    readings["weight"] = weight;
    readings["relayHeat"] = digitalRead(RELAY_HEAT);
    readings["relayFan"] = digitalRead(RELAY_FAN);

    String jsonString = JSON.stringify(readings);
    return jsonString;
}

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
        request->send(LittleFS, "/index.html", String(), false, processor);
    });

    // Route to set dryer status to FALSE
    server->on("/off0", HTTP_GET, [](AsyncWebServerRequest *request){
        setTareHX711();
        request->send(LittleFS, "/index.html", String(), false, processor);
    });

}

void card_1(AsyncWebServer *server) {

    // Route to set relay heater to LOW
    server->on("/on1", HTTP_GET, [](AsyncWebServerRequest *request){
        digitalWrite(RELAY_HEAT, LOW);
        display_addNotification("Heater ON");
        buzz_set(1000, 100, 100, 2);
        request->send(LittleFS, "/index.html", String(), false, processor);
    });

    // Route to set relay heater to HIGH
    server->on("/off1", HTTP_GET, [](AsyncWebServerRequest *request){
        digitalWrite(RELAY_HEAT, HIGH);
        display_addNotification("Heater OFF");
        buzz_set(1000, 100, 100, 2);
        request->send(LittleFS, "/index.html", String(), false, processor);
    });
}

void card_2(AsyncWebServer *server) {

    // Route to set relay fan to LOW
    server->on("/on2", HTTP_GET, [](AsyncWebServerRequest *request){
        digitalWrite(RELAY_FAN, LOW);
        display_addNotification("Fan ON");
        buzz_set(1000, 100, 100, 2);
        request->send(LittleFS, "/index.html", String(), false, processor);
    });

    // Route to set relay fan to HIGH
    server->on("/off2", HTTP_GET, [](AsyncWebServerRequest *request){
        digitalWrite(RELAY_FAN, HIGH);
        display_addNotification("Fan OFF");
        buzz_set(1000, 100, 100, 2);
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
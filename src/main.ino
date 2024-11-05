#include <Arduino.h>

// Project includes
#include "constants.h"
#include "sensors.h"
#include "wifi_manager.h"
#include "web_cards.h"

void setup() {
    Serial.begin(SERIAL_BAUD); delay(10);
    Serial.println("Project SMFDS [ALPHA]");Serial.println("Starting...");

    if (!beginSensors()) {
        Serial.println("Error initializing sensors");
    }
    else {
        Serial.println("Sensors initialized");
    }

    setupWebserver();
}

void loop() {
    readDHT(); 
    readHX711();
    //debug_randSensor();
    web_sendEvent();

    printSensors();
}
#include <Arduino.h>

// Project includes
#include "constants.h"
#include "util_timer.h"
#include "sensors.h"
#include "wifi_manager.h"
#include "web_cards.h"
#include "display_oled.h"

void setup() {
    Serial.begin(SERIAL_BAUD); delay(10);
    loadTimerState();

    Serial.println("Project SMFDS [ALPHA]");Serial.println("Starting...");

    if (!beginSensors()) {
        Serial.println("Error initializing sensors");
    }
    else {
        Serial.println("Sensors initialized");
    }

    setupWebserver();
    display_begin();

    buzz_set(1000, 100, 100, 4);
}

void loop() {
    buzz_loop();

    static int rCount = 0;

    if (Serial.available()) {
        char ch = Serial.read();
        if (ch == 'r') {
            rCount++;
            if (rCount == 3) {
                Serial.println("Resetting ESP32...");
                ESP.restart();
            }
        } else if (ch == 'a') {
            addTimer(15000);
        } else if (ch == 's') {
            resetTimer();
        } else if (ch == 't') {
            setTareHX711();
        }
        else {
            rCount = 0; // Reset the count if any other character is received
        }
    }

    readDHT(); 
    readHX711();
    //debug_randSensor();

    display_loop();
    web_sendEvent();
    // Serial
    printSensors();
}
#include <Arduino.h>

// Project includes
#include "constants.h"
#include "web_wifiManager.h"
#include "web_cards.h"
#include "util_timer.h"
#include "gpio_sensors.h"
#include "gpio_actuators.h"
#include "util_display.h"

void setup() {
    Serial.begin(SERIAL_BAUD); delay(10);
    Serial.println("Project SMFDS [ALPHA]");Serial.println("Starting...");
    
    beginWifiManager();
    loadTimerState();

    // initialize sensors and actuators
    if (!beginActuators()) {
        Serial.println("Error initializing actuators");
    }
    else {
        Serial.println("Actuators initialized");
    }
    
    if (!beginSensors() && !beginSensors_pow()) {
        Serial.println("Error initializing sensors");
    }
    else {
        Serial.println("Sensors initialized");
    }
    
    beginDisplay();

    setupBuzz(1000, 100, 250, 3); // Buzz to indicate startup
}

void loop() {
    loopWifiManager();
    loopBuzz();

    static int rCount = 0;
    bool opt = 0;

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
        } else if (ch == 'c') {
            opt = !opt;
        }
        else {
            rCount = 0; // Reset the count if any other character is received
        }
    }

    if (Serial) {
        //debug
        debug_randSensor();
        if (opt) printSensors();
    
    } else { 
        readSensors();
        readPower();
    }

    display_loop();
}
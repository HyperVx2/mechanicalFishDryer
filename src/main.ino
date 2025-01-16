#include <Arduino.h>

// Project includes
#include "constants.h"
#include "web_wifiManager.h"
#include "web_cards.h"
#include "util_timer.h"
#include "gpio_sensors.h"
#include "gpio_actuators.h"
#include "util_display.h"

const unsigned long interval_2 = 2000, interval_1 = 1000; // 2 seconds and 1 second
unsigned long previousMillis_2 = 0, previousMillis_1 = 0;

static int rCount = 0;
static bool opt = false;

void setup() {
    Serial.begin(SERIAL_BAUD);
    Serial.println("Project SMFDS [ALPHA]");Serial.println("Starting...");

    beginDisplay();

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

    setupBuzz(1000, 100, 250, 3); // Buzz to indicate startup
}

void SerialEvent() {

    if (Serial.available()) {
        char ch = Serial.read();
        if (ch == 'q') {
            rCount++;
            if (rCount == 3) {
                Serial.println("Resetting ESP32...");
                ESP.restart();
            }
        } else if (ch == 'a') {
            setTimer(15000);
        } else if (ch == 's') {
            setTimer(15000, true);
        } else if (ch == 'd') {
            resetTimer();
        } else if (ch == 'w') {
            setTareHX711();
        } else if (ch == 'e') {
            toggleActuator("heater", opt);
        } else if (ch == 'r') {
            toggleActuator("fan", opt);
        } else if (ch == '3') {
            opt = !opt;
        } else {
            rCount = 0; // Reset the count if any other character is received
        }
    }
}

void loop() {
    unsigned long currentMillis = millis();

    loopBuzz();
    //SerialEvent();

    display_loop();

    // 1 second timer
    if (currentMillis - previousMillis_1 >= interval_1) {
        previousMillis_1 = currentMillis;

        loopWifiManager();
    }

    // 2 seconds timer
    if (currentMillis - previousMillis_2 >= interval_2) {
        previousMillis_2 = currentMillis;

        readSensors();
        readPower();
        //debug_randSensor();
        
        if (opt) printSensors();
    }
}
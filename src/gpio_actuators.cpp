#include "gpio_actuators.h"

int buzzCount = 0;
bool buzzing = false;
int buzzInterval, buzzTimes, buzzFrequency, buzzDuration;

unsigned long buzz_lastTime;

// Initialize the LEDs, buzzer, and relay modules
bool beginActuators() {
    // Initialize LEDs and buzzer
    pinMode(LED_1, OUTPUT);
    pinMode(LED_2, OUTPUT);
    pinMode(LED_3, OUTPUT);
    pinMode(BUZZER, OUTPUT);

    // Turn on LED light
    digitalWrite(LED_1, HIGH);
    digitalWrite(LED_2, HIGH);
    digitalWrite(LED_3, HIGH);
    delay(1000);
    
    // Initialize relay modules and set to off
    pinMode(RELAY_HEAT, OUTPUT);
    pinMode(RELAY_FAN, OUTPUT);
    digitalWrite(RELAY_HEAT, HIGH);
    digitalWrite(RELAY_FAN, HIGH);

    // Turn back off LED light
    digitalWrite(LED_1, LOW);
    digitalWrite(LED_2, LOW);
    digitalWrite(LED_3, LOW);

    return true;
}

// Toggle a relay module of either the heater or fan
void toggleActuator(String relay, bool state) {
    if (relay == "heater") {
        digitalWrite(RELAY_HEAT, state);
        digitalWrite(LED_1, state);
    } else if (relay == "fan") {
        digitalWrite(RELAY_FAN, state);
    }
}

/// @brief Setup the buzzer.
/// @param frequency The frequency of the buzzer sound in Hertz (Hz).
/// @param duration The duration for which the buzzer should sound in milliseconds (ms).
/// @param interval The interval between buzzes in milliseconds (ms).
/// @param times The number of times the buzzer should sound.
void setupBuzz(int frequency, int duration, int interval, int times) {
    buzzFrequency = frequency;
    buzzDuration = duration;
    buzzInterval = interval;
    buzzTimes = times;
    buzzCount = 0;
    buzzing = true;
    buzz_lastTime = millis();
}

// Loop for the buzzer
void loopBuzz() {
    if (buzzing) {
        unsigned long currentMillis = millis();
        if (currentMillis - buzz_lastTime >= buzzInterval) {
            buzz_lastTime = currentMillis;
            buzzCount++;
            if (buzzCount > buzzTimes) {
                buzzing = false;
                noTone(BUZZER); // Ensure buzzer is off
            } else {
                tone(BUZZER, buzzFrequency, buzzDuration); // Continue buzzing
            }
        }
    }
}
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
    
    // Initialize relay modules and set to off
    pinMode(RELAY_HEAT, OUTPUT);
    pinMode(RELAY_FAN, OUTPUT);
    digitalWrite(RELAY_HEAT, HIGH);
    digitalWrite(RELAY_FAN, HIGH);

    return true;
}

// Set the buzzer
void buzz_set(int frequency, int duration, int interval, int times) {
    buzzFrequency = frequency;
    buzzDuration = duration;
    buzzInterval = interval;
    buzzTimes = times;
    buzzCount = 0;
    buzzing = true;
    buzz_lastTime = millis();
}

// Loop for the buzzer
void buzz_loop() {
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
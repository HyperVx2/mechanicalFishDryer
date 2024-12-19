#include "gpio_actuators.h"
#include "util_display.h" // Assuming display_addNotification() is defined here

int buzzCount = 0;
bool buzzing = false;
int buzzInterval, buzzTimes, buzzFrequency, buzzDuration;
unsigned long buzz_lastTime;

// Buzzer parameters
const int BUZZ_ON_FREQ = 700;
const int BUZZ_OFF_FREQ = 400;
const int BUZZ_DURATION = 100;
const int BUZZ_INTERVAL = 200;
const int BUZZ_TIMES = 2;

// Relay names
const String RELAY_H = "heater";
const String RELAY_F = "fan";

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

// Toggle a relay module of either the heater or fan
void toggleActuator(const String& relay, bool state) {
    int relayPin, ledPin = -1;
    String msg;

    if (relay == RELAY_H) {
        relayPin = RELAY_HEAT;
        ledPin = LED_3;
        msg = "Heater ";
    } else if (relay == RELAY_F) {
        relayPin = RELAY_FAN;
        msg = "Fan ";
    } else {
        return; // Invalid relay name
    }

    digitalWrite(relayPin, state ? LOW : HIGH);
    if (ledPin != -1) {
        digitalWrite(ledPin, state ? HIGH : LOW);
    }

    msg += (state ? "ON" : "OFF");
    display_addNotification(msg);

    // Setup the buzzer to indicate the relay state change
    setupBuzz(state ? BUZZ_ON_FREQ : BUZZ_OFF_FREQ, BUZZ_DURATION, BUZZ_INTERVAL, BUZZ_TIMES);
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
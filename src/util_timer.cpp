#include "util_timer.h"

Preferences preferences;

// Timer variables
unsigned long timerDuration = 0; // Duration of the timer in milliseconds
unsigned long timerStartMillis = 0; // When the timer started
bool timerRunning = false;

// Buzzer parameters
const int BUZZ_START_FREQ = 800;
const int BUZZ_START_DURATION = 100;
const int BUZZ_START_INTERVAL = 200;
const int BUZZ_START_TIMES = 3;

const int BUZZ_EXTEND_FREQ = 500;
const int BUZZ_EXTEND_DURATION = 100;
const int BUZZ_EXTEND_INTERVAL = 200;
const int BUZZ_EXTEND_TIMES = 2;

const int BUZZ_RESET_FREQ = 500;
const int BUZZ_RESET_DURATION = 100;
const int BUZZ_RESET_INTERVAL = 200;
const int BUZZ_RESET_TIMES = 2;

const int BUZZ_EXPIRE_FREQ = 800;
const int BUZZ_EXPIRE_DURATION = 500;
const int BUZZ_EXPIRE_INTERVAL = 1000;
const int BUZZ_EXPIRE_TIMES = 4;

// Function to save the timer state
void saveTimerState() {
  preferences.begin("timer", false); // Open NVS namespace
  preferences.putULong("duration", timerDuration);
  preferences.putULong("startMillis", millis() - timerStartMillis); // Save elapsed time
  preferences.putBool("running", timerRunning);
  preferences.end();
}

// Function to load the timer state
void loadTimerState() {
  preferences.begin("timer", true); // Open NVS namespace in read-only mode
  timerDuration = preferences.getULong("duration", 0); // Default to 0 if not found
  unsigned long elapsedTime = preferences.getULong("startMillis", 0);
  timerRunning = preferences.getBool("running", false);
  preferences.end();

  // Restore start time based on the saved elapsed time
  if (timerRunning) {
    timerStartMillis = millis() - elapsedTime;
  }
}

// Function to start or extend the timer
void setTimer(unsigned long duration, bool extend) {
  if (extend) {
    timerDuration += duration;
    setupBuzz(BUZZ_EXTEND_FREQ, BUZZ_EXTEND_DURATION, BUZZ_EXTEND_INTERVAL, BUZZ_EXTEND_TIMES);
    Serial.print("Timer extended by ");
  } else {
    timerDuration = duration;
    setupBuzz(BUZZ_START_FREQ, BUZZ_START_DURATION, BUZZ_START_INTERVAL, BUZZ_START_TIMES);
    Serial.print("Timer started for ");
  }
  timerStartMillis = millis();
  timerRunning = true;
  saveTimerState(); // Save the state

  Serial.print(duration); Serial.println("ms");
}

// Function to check the timer
unsigned long getRemainingTime() {
  if (!timerRunning) {
    return 0;
  }

  unsigned long elapsed = millis() - timerStartMillis;
  saveTimerState(); // Save state

  if (elapsed >= timerDuration) {
    timerRunning = false;
    timerDuration = 0;
    timerStartMillis = 0;

    digitalWrite(LED_3, LOW);
    setupBuzz(BUZZ_EXPIRE_FREQ, BUZZ_EXPIRE_DURATION, BUZZ_EXPIRE_INTERVAL, BUZZ_EXPIRE_TIMES);
    return 0; // Timer expired
  }

  return timerDuration - elapsed;
}

// Function to reset the timer
void resetTimer() {
  timerDuration = 0;
  timerStartMillis = 0;
  timerRunning = false;
  saveTimerState(); // Save reset state

  Serial.println("Timer reset");
  setupBuzz(BUZZ_RESET_FREQ, BUZZ_RESET_DURATION, BUZZ_RESET_INTERVAL, BUZZ_RESET_TIMES);
}
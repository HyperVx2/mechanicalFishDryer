#include "util_timer.h"

Preferences preferences;

// Timer variables
unsigned long timerDuration = 0; // Duration of the timer in milliseconds
unsigned long timerStartMillis = 0; // When the timer started
bool timerRunning = false;

// Function to save the timer state
void saveTimerState() {
  preferences.begin("timer", false); // Open NVS namespace
  preferences.putULong("duration", timerDuration);
  preferences.putULong("startMillis", millis() - timerStartMillis); // Save elapsed time
  preferences.putBool("running", timerRunning);
  preferences.end();

  Serial.println("Timer state saved");
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

  Serial.println("Timer state loaded");
}

// Function to start the timer
void startTimer(unsigned long duration) {
  timerDuration = duration;
  timerStartMillis = millis();
  timerRunning = true;
  saveTimerState(); // Save the state

  Serial.print("Timer started for "); Serial.print(duration); Serial.println("ms");
}

// Function to add time to the timer
void addTimer(unsigned long duration) {
  timerDuration += duration;
  timerStartMillis = millis();
  timerRunning = true;
  saveTimerState(); // Save the state

  Serial.print("Timer extended by "); Serial.print(duration); Serial.println("ms");
}

// Function to check the timer
unsigned long getRemainingTime() {
    unsigned long elapsed = millis() - timerStartMillis;
    if (timerRunning) {
        if (elapsed >= timerDuration) {
            timerRunning = false;
            timerDuration = 0;
            saveTimerState(); // Save final state
            
            return 0; // Timer expired
        }
    }
    
    // Get the current timer duration
    unsigned long remainingTime = timerDuration - elapsed;
    
    // Call saveTimerState()
    saveTimerState();
    
    return remainingTime;
}

// Function to reset the timer
void resetTimer() {
  timerDuration = 0;
  timerStartMillis = 0;
  timerRunning = false;
  saveTimerState(); // Save reset state

  Serial.println("Timer reset");
}
#ifndef UTIL_TIMER
#define UTIL_TIMER

#include <Arduino.h>
#include <Preferences.h>

#include "constants.h"
#include "gpio_sensors.h"

extern unsigned long timerDuration, timerStartMillis;
extern bool timerRunning;

void loadTimerState();
void setTimer(unsigned long duration, bool extend = false);
unsigned long getRemainingTime();
void resetTimer();

#endif
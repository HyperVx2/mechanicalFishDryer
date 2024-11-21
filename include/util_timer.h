#ifndef UTIL_TIMER
#define UTIL_TIMER

#include <Preferences.h>

#include "constants.h"

extern unsigned long timerDuration, timerStartMillis;
extern bool timerRunning;

void loadTimerState();
void startTimer(unsigned long duration);
void addTimer(unsigned long duration);
unsigned long getRemainingTime();
void resetTimer();

#endif
#ifndef GPIO_ACTUATORS
#define GPIO_ACTUATORS

#include <Arduino.h>

#include "constants.h"
#include "util_display.h"

bool beginActuators();

void toggleActuator(String relay, bool state);

void setupBuzz(int frequency, int duration, int interval, int times);
void loopBuzz();

#endif
#ifndef GPIO_ACTUATORS
#define GPIO_ACTUATORS

#include <Arduino.h>

#include "constants.h"
#include "util_display.h"

bool beginActuators();

void buzz_set(int frequency, int duration, int interval, int times);
void buzz_loop();

#endif
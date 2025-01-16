#ifndef UTIL_DISPLAY
#define UTIL_DISPLAY

#include <SPI.h>
#include <Wire.h>
#include <queue>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

#include "constants.h"
#include "util_timer.h"
#include "gpio_sensors.h"
#include "gpio_actuators.h"
#include "web_wifiManager.h"

void display_addNotification(String text);
String formatTime(unsigned long duration);

void beginDisplay();
void display_loop();

#endif
#ifndef DISPLAY_OLED
#define DISPLAY_OLED

#include <SPI.h>
#include <Wire.h>
#include <queue>
#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>

#include "constants.h"
#include "util_timer.h"
#include "sensors.h"
#include "wifi_manager.h"

void display_addNotification(String text);
String formatTime(unsigned long duration);

void display_begin();
void display_loop();

#endif
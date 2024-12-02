#ifndef SENSORS_H
#define SENSORS_H

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <HX711_ADC.h>
#include <EEPROM.h>
#include <ACS712.h>

#include "constants.h"
#include "display_oled.h"

extern float temperature, humidity, weight;
extern float current, voltage;

bool beginSensors();
bool beginSensors_2();

void buzz_set(int frequency, int duration, int interval, int times);
void buzz_loop();

void readDHT();
void readHX711();
void debug_randSensor();

void printSensors();

void setTareHX711();

#endif
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

bool beginSensors();

void readDHT();
void readHX711();
void debug_randSensor();

void setTareHX711();

#endif
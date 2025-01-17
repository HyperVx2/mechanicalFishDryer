#ifndef GPIO_SENSORS
#define GPIO_SENSORS

#include <Arduino.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <HX711_ADC.h>
#include <ACS712.h>
#include <ZMPT101B.h>

#include "constants.h"
#include "util_display.h"
#include "gpio_actuators.h"

extern float temperature, humidity, weight;
extern float current, voltage;

bool beginSensors();
bool beginSensors_pow();

void readSensors();
void readPower();
void debug_randSensor();

void printSensors();

void setTareHX711();

#endif
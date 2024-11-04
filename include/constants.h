/*
    Usable ESP32 pins (in order): 14, 27, 26, 25, 33, 32
    Ref: https://lastminuteengineers.com/esp32-pinout-reference

    Assigned pin (in order): RELAY_HEATER, HX711_DT, HX711_SCK, DHT22, RELAY_FAN, DHT11_CONTROL

    Ref 1 (dht22): https://github.com/adafruit/DHT-sensor-library/blob/master/examples/DHTtester/DHTtester.ino
    Ref 2 (hx711): https://github.com/olkal/HX711_ADC/blob/master/examples/Read_1x_load_cell_interrupt_driven/Read_1x_load_cell_interrupt_driven.ino
    Ref 3 (scheduler): https://www.norwegiancreations.com/2017/09/arduino-tutorial-using-millis-instead-of-delay/
    Ref 4 (dashboard): https://github.com/lkoepsel/Dashboard
*/

#ifndef CONSTANTS_H
#define CONSTANTS_H

#define COMPANY_NAME "Team Early Workers"
#define PRODUCT_NAME "SMFDS"
#define PRODUCT_DESCRIPTION "A smart mechanical fish drying system using various sensors and fuzzy logic."
#define GITHUB_URL "https://github.com/HyperVx2/mechanicalfishdryer"

// Serial
#define SERIAL_BAUD 9600
#define DELAY_MS 1000

// WiFi and Web Server
#define WIFI_SSID "SMFDS_ESP32"
#define WIFI_PASS "foobar123"
#define WEB_PORT 80

// Relay Module
#define RELAY_HEAT 33
#define RELAY_FAN 14

// Sensor: DHT22 Temperature and Humidity
#define DHT_PIN 25
#define DHT_TYPE DHT22

// Sensor: HX711 Weight Sensor Amplifier
#define HX711_DOUT 27
#define HX711_SCK 26
#define HX711_CALIBRATION 1500

// Sensor: ACS712 (30A) Current Sensor
#define ACS712_PIN 00 //TODO

// Sensor: ZMPT101B Voltage Sensor
#define ZMPT101B_PIN 00 //TODO

#endif
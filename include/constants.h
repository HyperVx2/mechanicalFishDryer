/*
    Ref: https://lastminuteengineers.com/esp32-pinout-reference

    PIN ASSIGNMENT:
    LED_1 (RED):    D23 | LED_2 (GREEN):    D19 | LED_3 (RED):  D18
    BUZZER:         D32
    RELAY_HEATER:   D14 | RELAY_FAN:        D33
    OLED_SCL:       D22 | OLED_SDA:         D21
    DHT22:          D25 | HX711_DT:         D27 | HX711_SCK:    D26
    ACS712:         D16 | ZMPT101B:         D17

    LED ASSIGNMENT:
    LED_1 (RED) = timerRunning | LED_2 (GREEN) = remainingTime | LED_3 (RED) = RELAY_HEATER

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

// EEPROM Addresses
#define TIMER_START_ADDR 0
#define TIMER_DURATION_ADDR (TIMER_START_ADDR + sizeof(unsigned long))
#define TIMER_REMAINING_ADDR (TIMER_DURATION_ADDR + sizeof(unsigned long))
#define SYSTEM_IDLE_ADDR (TIMER_REMAINING_ADDR + sizeof(unsigned long))
#define HX711_CALVAL_ADDR (SYSTEM_IDLE_ADDR + sizeof(bool))

// WiFi and Web Server
#define WIFI_SSID "SMFDS_ESP32"
#define WIFI_PASS "foobar123"
#define WEB_PORT 80

// LED, buzzer, & relay
#define LED_1 23 // Red
#define LED_2 19 // Green
#define LED_3 18 // Red
#define BUZZER 32 
#define RELAY_HEAT 14
#define RELAY_FAN 33

// Sensor: DHT22 Temperature and Humidity
#define DHT_PIN 25
#define DHT_TYPE DHT22

// Sensor: HX711 Weight Sensor Amplifier
#define HX711_DOUT 27
#define HX711_SCK 26
#define HX711_CALIBRATION 1500

// Sensor: ACS712 (30A) Current Sensor
#define ACS712_PIN 16
#define ACS712_VOLT 5.0
#define ACS712_ADC 1023
#define ACS712_mvPerA 66

// Sensor: ZMPT101B Voltage Sensor
#define ZMPT101B_PIN 17

#endif
/*
    Usable ESP32 pins (in order): 14, 27, 26, 25, 33, 32
    Ref: https://lastminuteengineers.com/esp32-pinout-reference

    Assigned pin (in order): RELAY_HEATER, HX711_DT, HX711_SCK, DHT22, RELAY_FAN, DHT11_CONTROL

    Ref 1 (dht22): https://github.com/adafruit/DHT-sensor-library/blob/master/examples/DHTtester/DHTtester.ino
    Ref 2 (hx711): https://github.com/olkal/HX711_ADC/blob/master/examples/Read_1x_load_cell_interrupt_driven/Read_1x_load_cell_interrupt_driven.ino
    Ref 3 (scheduler): https://www.norwegiancreations.com/2017/09/arduino-tutorial-using-millis-instead-of-delay/
    Ref 4 (dashboard): https://github.com/lkoepsel/Dashboard
*/

#include <Arduino.h>
#include <DHT.h>
#include <HX711_ADC.h>
#include <EEPROM.h>
#include <LittleFS.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>

#include "serve.h"
#include "card_0.h"
#include "card_1.h"
#include "card_2.h"
#include "card_3.h"

// Serial Port Constants and Variables
#define SERIAL_BAUD 921600

// scheduler code
unsigned int oneSecPeriod = 1000; // 1sec reading
unsigned long time_now = 0;

// Relay code
#define relay_heat 33
#define relay_fan 14
bool dryerStatus = false;

// DHT22 code
#define DHTPIN 25
DHT dht(DHTPIN, DHT22); // using DHT22 sensor
float humidity, temperature; //humidity, temperature data

// HX711_ADC code
const int HX711_dout = 27;
const int HX711_sck = 26;
HX711_ADC loadCell(HX711_dout, HX711_sck);

const int calVal_eepromAddress = 0;
unsigned long t = 0;
volatile boolean weight_newDataReady;

float weightCalValue = 258.80; // todo: change value
float weight; // weight data

// Access Point Constants and Variables
const char *ssid = "SMFDS_ESP32";
const char *password = "foobar123";

// Create AsyncWebServer 
#define WEB_PORT 80
AsyncWebServer server(WEB_PORT);

// Create an Event Source on /events
AsyncEventSource events("/events");

void setup() {
    Serial.begin(SERIAL_BAUD); delay(10);
    Serial.println(); Serial.println("Starting...");

    // Initialize SPIFFS
    if(!LittleFS.begin(true)) {
        Serial.println("An Error has occured while mounting LittleFS");
        return;
    }

    // init relay module
    pinMode(relay_heat, OUTPUT);
    pinMode(relay_fan, OUTPUT);
    digitalWrite(relay_heat, HIGH);
    digitalWrite(relay_fan, HIGH);

    // init hx711/dht22
    EEPROM.begin(512); // fetch the calibration value from eeprom
    unsigned long stabilizingTime = 2000;
    boolean _tare = true;  //set this to false if you don't want tare to be performed in the next step

    dht.begin();
    loadCell.begin();

    loadCell.start(stabilizingTime, _tare);
    if (isnan(humidity) || isnan(temperature)) {
        Serial.println(F("Timeout, check DHT22 wiring and pin designations."));
        while (1);
    }
    if (loadCell.getTareTimeoutFlag()) {
        Serial.println("Timeout, check MCU>HX711 wiring and pin designations.");
        while (1);
    } else {
        loadCell.setCalFactor(weightCalValue);
        Serial.println("Startup is complete.");
    }

    attachInterrupt(digitalPinToInterrupt(HX711_dout), dataReadyISR, FALLING);

    // Initialize WIFI AP
    WiFi.softAP(ssid, password);
    IPAddress myIP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(myIP);

    events.onConnect([](AsyncEventSourceClient *client){
      if(client->lastId()){
        Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
      }
      // send event with message "hello!", id current millis
      // and set reconnect delay to 1 second
      client->send("hello!", NULL, millis(), 10000);
    });
    server.addHandler(&events);

    Serial.println("Server started.");
    serve(&server);
    server.begin();
}

// interrupt routune for HX711
void dataReadyISR() {
    if (loadCell.update()) {
        weight_newDataReady = 1;
    }
}

void loop() {
    if(millis() >= time_now + oneSecPeriod) { 
        //temperature = dht.readTemperature();
        //humidity = dht.readHumidity();

        if (weight_newDataReady) {
            weight = loadCell.getData();
            weight_newDataReady = 0;
        }

        //serialPlotData();
        time_now = millis();
    }

    // check if last tare operation is complete
    if (loadCell.getTareStatus() == true) {
        Serial.println("Tare complete");
    }
}

void serialPlotData() {
    Serial.print(F(">humidity:"));
    Serial.println(humidity);
    Serial.print(F(">temperature:"));
    Serial.println(temperature);

    Serial.print(">weight:");
    Serial.println(weight);
}
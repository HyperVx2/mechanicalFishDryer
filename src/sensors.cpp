#include "sensors.h"

float temperature, humidity, weight;
float current, voltage;

unsigned long dht_lastTime;
unsigned long hx711_lastTime;
unsigned long randSensor_lastTime;

uint32_t DHT_delayMS;

DHT_Unified dht(DHT_PIN, DHT_TYPE);
HX711_ADC hx711(HX711_DOUT, HX711_SCK);

volatile boolean HX711_newDataReady;

//interrupt routine:
void HX711_dataReadyISR() {
  if (hx711.update()) {
    HX711_newDataReady = true;
  }
}

ACS712 ACS(ACS712_PIN, ACS712_VOLT, ACS712_ADC, ACS712_mvPerA);


bool beginSensors() {
    // Initialize actuators
    pinMode(RELAY_HEAT, OUTPUT);
    pinMode(RELAY_FAN, OUTPUT);
    digitalWrite(RELAY_HEAT, HIGH);
    digitalWrite(RELAY_FAN, HIGH);

    // Initialize DHT22
    dht.begin();
    sensor_t DHTsensor;
    dht.temperature().getSensor(&DHTsensor);
    dht.humidity().getSensor(&DHTsensor);
    DHT_delayMS = DHTsensor.min_delay / 1000;

    // initialize HX711
    float calibrationValue = HX711_CALIBRATION;
    EEPROM.begin(512);
    EEPROM.get(HX711_CALVAL_ADDR, calibrationValue); // Fetch the value from eeprom

    hx711.begin();
    unsigned long stabilizingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilizing time
    boolean _tare = false; //set this to false if you don't want tare to be performed in the next step
    hx711.start(stabilizingtime, _tare);
    if (hx711.getTareTimeoutFlag()) {
        Serial.println("Timeout, check MCU>HX711 wiring and pin designations");
    }
    else {
        hx711.setCalFactor(calibrationValue); // set calibration value (float)
        Serial.println("HX711 setup complete.");
    }

    attachInterrupt(digitalPinToInterrupt(HX711_DOUT), HX711_dataReadyISR, FALLING);

    return true;
}

bool beginSensors_2() {
    // Initialize ACS712
    ACS712 ACS(ACS712_PIN, ACS712_VOLT, ACS712_ADC, ACS712_mvPerA);
    ACS.autoMidPoint();
    Serial.print("MidPoint: ");
    Serial.print(ACS.getMidPoint());
    Serial.print(". Noise mV: ");
    Serial.println(ACS.getNoisemV());
    return true;
}

void readDHT() {
    sensors_event_t event;
    if (millis() > dht_lastTime + DHT_delayMS) {
        // Get temperature event and print its value.
        dht.temperature().getEvent(&event);
        if (isnan(event.temperature)) {
            Serial.println(F("Error reading temperature!"));
        } else {
            temperature = event.temperature;
        }
        // Get humidity event and print its value.
        dht.humidity().getEvent(&event);
        if (isnan(event.relative_humidity)) {
            Serial.println(F("Error reading humidity!"));
        } else {
            humidity = event.relative_humidity;
        }
            dht_lastTime = millis();
    }
}

void readHX711() {
    // get smoothed value from the dataset:
    if (HX711_newDataReady) {
        if (millis() > hx711_lastTime + DELAY_MS) {
        weight = hx711.getData();
        HX711_newDataReady = false;
        hx711_lastTime = millis();
        }
    }
}

void setTareHX711() {
    hx711.tareNoDelay();
    if (hx711.getTareStatus() == true) {
        Serial.println("Tare complete");
        //return true;
    } else {
        Serial.println("Tare timeout, check MCU>HX711 wiring and pin designations");
        //return false;
    }
}

void readACS712() {
    current = ACS.mA_AC();
    voltage = ACS.getMidPoint();
}

void printSensors() {
    Serial.print("Temp: "); Serial.println(temperature);
    Serial.print("Humidity: "); Serial.println(humidity);
    Serial.print("Weight: "); Serial.println(weight);
}

void debug_randSensor() { 
    if (millis() > randSensor_lastTime + DELAY_MS) {
        temperature = random(30, 80);
        humidity = random(30, 99);
        weight = random(-120, 1200);
        randSensor_lastTime = millis();
    }
}
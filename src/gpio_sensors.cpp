#include "gpio_sensors.h"

float temperature, humidity, weight;
float calibrationValue = HX711_CALIBRATION;
float current, voltage;

unsigned long dht_lastTime;
unsigned long hx711_lastTime;
unsigned long randSensor_lastTime;
unsigned long printSensor_lastTime;

uint32_t DHT_delayMS;

DHT_Unified dht(DHT_PIN, DHT_TYPE);
HX711_ADC hx711(HX711_DOUT, HX711_SCK);

volatile boolean HX711_newDataReady;

// (HX711) interrupt routine:
void HX711_dataReadyISR() {
  if (hx711.update()) {
    HX711_newDataReady = 1;
  }
}

ACS712 ACS(ACS712_PIN, ACS712_VOLT, ACS712_ADC, ACS712_mvPerA);

// Initialize DHT22 and HX711
bool beginSensors() {
    // Initialize DHT22
    dht.begin();
    sensor_t DHTsensor;
    dht.temperature().getSensor(&DHTsensor);
    dht.humidity().getSensor(&DHTsensor);
    DHT_delayMS = DHTsensor.min_delay / 1000;

    // initialize HX711
    hx711.begin();
    unsigned long stabilizingtime = 2000; // tare preciscion can be improved by adding a few seconds of stabilizing time
    boolean _tare = true; //set this to false if you don't want tare to be performed in the next step
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


// Initialize ACS712 and ZMT101B
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
        HX711_newDataReady = 0;
        hx711_lastTime = millis();
        }
    }

    if (hx711.getTareStatus() == true) {
        Serial.println("Tare complete");
        display_addNotification("Tare set");
        buzz_set(1000, 500, 1000, 1);
    }
}

void setTareHX711() {
    hx711.tareNoDelay();
}

void readACS712() {
    current = ACS.mA_AC();
    voltage = ACS.getMidPoint();
}

void printSensors() {
    if (millis() > printSensor_lastTime + 3000) {
        Serial.print(">temperature:"); Serial.println(temperature);
        Serial.print(">humidity:"); Serial.println(humidity);
        Serial.print(">weight:"); Serial.println(weight);
        Serial.print(">current:"); Serial.println(current);
        Serial.print(">voltage:"); Serial.println(voltage);
        Serial.println();

        printSensor_lastTime = millis();
    }
}

void debug_randSensor() { 
    if (millis() > randSensor_lastTime + DELAY_MS) {
        temperature = random(30, 80);
        humidity = random(30, 99);
        weight = random(-120, 1200);
        randSensor_lastTime = millis();
    }
}
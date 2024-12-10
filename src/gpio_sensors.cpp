#include "gpio_sensors.h"

float calibrationValue = HX711_CALIBRATION;
float temperature, humidity, weight;
float current, voltage, power;

unsigned long dht_lastTime;
unsigned long hx711_lastTime;
unsigned long randSensor_lastTime;
unsigned long printSensor_lastTime;

uint32_t DHT_delayMS;

DHT_Unified dht(DHT_PIN, DHT_TYPE);
HX711_ADC hx711(HX711_DOUT, HX711_SCK);
ACS712 ACS(ACS712_PIN, ACS712_VOLT, ACS712_ADC, ACS712_mvPerA);
ZMPT101B voltageSensor(ZMPT101B_PIN);

volatile boolean HX711_newDataReady;

// (HX711) interrupt routine:
void HX711_dataReadyISR() {
  if (hx711.update()) {
    HX711_newDataReady = 1;
  }
}

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
bool beginSensors_pow() {
    // Initialize ACS712
    ACS.autoMidPoint();
    Serial.print("MidPoint: ");
    Serial.print(ACS.getMidPoint());
    Serial.print(". Noise mV: ");
    Serial.println(ACS.getNoisemV());

    // Initialize ZMT101B
    voltageSensor.calibrate();
    return true;
}

// Read DHT22 and HX711 data
void readSensors() {
    sensors_event_t event;
    if (millis() > dht_lastTime + DELAY_MS) {
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

    // get smoothed value from the dataset
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
        setupBuzz(500, 100, 200, 2);
    }
}

// Set tare for HX711
void setTareHX711() {
    hx711.tareNoDelay();
}

// Define states for the state machine
enum ReadACS712State {
    IDLE,
    READING,
    DONE
};

ReadACS712State readState = IDLE;
float average = 0;
int readCount = 0;
unsigned long startTime = 0;
const int numReadings = 100;

void readPower() {
    switch (readState) {
        case IDLE:
            average = 0;
            readCount = 0;
            startTime = millis();
            readState = READING;
            break;

        case READING:
            if (readCount < numReadings) {
                average += ACS.mA_AC();
                readCount++;
            } else {
                current = average / numReadings;
                voltage = voltageSensor.getVoltageAC();
                power = current * voltage;
                readState = DONE;
            }
            break;

        case DONE:
            // Reading is complete, reset state to IDLE for next call
            readState = IDLE;
            break;
    }
}

void printSensors() {
    if (millis() > printSensor_lastTime + 3000) {
        Serial.print(">t:"); Serial.println(temperature);
        Serial.print(">h:"); Serial.println(humidity);
        Serial.print(">w:"); Serial.println(weight);
        Serial.print(">c:"); Serial.println(current);
        Serial.print(">v:"); Serial.println(voltage);
        Serial.print(">p:"); Serial.println(power);
        Serial.println();

        printSensor_lastTime = millis();
    }
}

void debug_randSensor() { 
    if (millis() > randSensor_lastTime + DELAY_MS) {
        temperature = random(30, 80);
        humidity = random(30, 99);
        weight = random(-120, 1200);
        current = 30;
        voltage = 230;
        power = current * voltage;
        randSensor_lastTime = millis();
    }
}
#include "gpio_sensors.h"

float calibrationValue = HX711_CALIBRATION;
float temperature, humidity, weight;
float current, voltage;

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

void readSensors() {
    // Get temperature and humidity event
    sensors_event_t tempEvent, humidityEvent;
    dht.temperature().getEvent(&tempEvent);
    dht.humidity().getEvent(&humidityEvent);

    temperature = isnan(tempEvent.temperature) ? -1 : round(tempEvent.temperature * 100) / 100.0;
    humidity = isnan(humidityEvent.relative_humidity) ? -1 : round(humidityEvent.relative_humidity * 100) / 100.0;

    // Get smoothed weight value from the dataset
    if (HX711_newDataReady) {
        weight = round(hx711.getData() * 100) / 100.0;
        HX711_newDataReady = 0;
    }

    // Check tare status
    if (hx711.getTareStatus()) {
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

void readPower() {
    static float average = 0;
    static int readCount = 0;
    static unsigned long startTime = 0;
    const int numReadings = 100;

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
    Serial.print(">t:"); Serial.println(temperature, 2);
    Serial.print(">h:"); Serial.println(humidity, 2);
    Serial.print(">w:"); Serial.println(weight, 2);
    Serial.print(">c:"); Serial.println(current);
    Serial.print(">v:"); Serial.println(voltage);
    Serial.println();
}

void debug_randSensor() { 
    temperature = random(30, 80);
    humidity = random(30, 99);
    weight = random(-120, 1200);
    current = 30;
    voltage = 230;
}
/*
    Usable ESP32 pins (in order): 14, 27, 26, 25, 33, 32
    Ref: https://lastminuteengineers.com/esp32-pinout-reference

    Assigned pin (in order): RELAY_HEATER, HX711_DT, HX711_SCK, DHT22, RELAY_FANq, DHT11_CONTROL

    Ref 1 (dht22): https://github.com/adafruit/DHT-sensor-library/blob/master/examples/DHTtester/DHTtester.ino
    Ref 2 (hx711): https://github.com/olkal/HX711_ADC/blob/master/examples/Read_1x_load_cell_interrupt_driven/Read_1x_load_cell_interrupt_driven.ino
    Ref 3 (scheduler): https://www.norwegiancreations.com/2017/09/arduino-tutorial-using-millis-instead-of-delay/
*/

#include <DHT.h>
#include <HX711_ADC.h>
#include <EEPROM.h>

/* scheduler code */
const int period = 1000; // in ms
unsigned long time_now = 0;

/* Relay code */
#define relay_heat 33
#define relay_fan 14

/* DHT22 code */
#define DHTPIN 25
DHT dht(DHTPIN, DHT22); // using DHT22 sensor

float hum = 0, tem = 0; //humidity, temperature data

/* HX711_ADC code */
const int HX711_dout = 27;
const int HX711_sck = 26;
HX711_ADC loadCell(HX711_dout, HX711_sck);

const int calVal_eepromAddress = 0;
unsigned long t = 0;
volatile boolean newDataReady;

float weightCalValue = 229.63; // todo: change value
float weight = 0; // weight data

void setup() {
    Serial.begin(9600); delay(10);
    Serial.println(); Serial.println("Starting...");

    pinMode(relay_heat, OUTPUT);
    pinMode(relay_fan, OUTPUT);

    //debug
    digitalWrite(relay_heat, HIGH);
    digitalWrite(relay_fan, HIGH);

    EEPROM.begin(512); // fetch the calibration value from eeprom
    unsigned long stabilizingTime = 2000;
    boolean _tare = true;  //set this to false if you don't want tare to be performed in the next step

    dht.begin();
    loadCell.begin();

    loadCell.start(stabilizingTime, _tare);
    if (isnan(hum) || isnan(tem)) {
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
}

/* interrupt routune for HX711 */
void dataReadyISR() {
    if (loadCell.update()) {
        newDataReady = 1;
    }
}

void loop() {
    hum = dht.readHumidity();
    tem = dht.readTemperature();
    
    if (newDataReady) {
        weight = loadCell.getData();
        newDataReady = 0;
    }

    if(millis() >= time_now + period){
        Serial.print(F(">humidity:"));
        Serial.println(hum);
        Serial.print(F(">temperature:"));
        Serial.println(tem);

        Serial.print(">weight:");
        Serial.println(weight);
        
        time_now = millis();
    }

    // receive comand from serial terminal
    if (Serial.available() > 0) {
        char inByte = Serial.read();

        switch(inByte) {
            case 't': // set tare
                loadCell.tareNoDelay();
                break;
            case 'i': // enable fan and heater
                digitalWrite(relay_heat, LOW);
                digitalWrite(relay_fan, LOW);
                Serial.println("Fan and heater turned ON.");
                break;
            case 'o': // disable fan and heater
                digitalWrite(relay_heat, HIGH);
                digitalWrite(relay_fan, HIGH);
                Serial.println("Fan and heater turned OFF.");
                break;
            case 'q':
                digitalWrite(relay_fan, LOW);
                Serial.println("Fan turned ON.");
                break;
        }
    }

    // check if last tare operation is complete
    if (loadCell.getTareStatus() == true) {
        Serial.println("Tare complete");
    }
}
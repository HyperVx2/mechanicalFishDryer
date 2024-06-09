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
#include <WiFi.h>

/* WiFi */
// network credentials
const char* ssid = "MFD_rPi4";
const char* password = "foobar123";

// softAP credentials
const char* softap_ssid = "MFD_ESP32";
const char* softap_password = "foobar123";

// IP Address details
IPAddress local_ip(10,42,0,10);
IPAddress gateway(10,42,0,1);
IPAddress softap_gateway(10,42,0,10);
IPAddress subnet(255,255,255,0);
IPAddress primaryDNS(1,1,1,1);
IPAddress secondaryDNS(1,0,0,1);

/* scheduler code */
unsigned int oneSecPeriod = 1000; // 1sec reading
unsigned long time_now = 0;

/* Relay code */
#define relay_heat 33
#define relay_fan 14

bool relay_heat_status = HIGH;
bool relay_fan_status = HIGH;

/* DHT22 code */
#define DHTPIN 25
DHT dht(DHTPIN, DHT22); // using DHT22 sensor

float hum, tem; //humidity, temperature data

/* HX711_ADC code */
const int HX711_dout = 27;
const int HX711_sck = 26;
HX711_ADC loadCell(HX711_dout, HX711_sck);

const int calVal_eepromAddress = 0;
unsigned long t = 0;
volatile boolean weight_newDataReady;

float weightCalValue = 258.80; // todo: change value
float weight; // weight data

/* start WiFi function */
void wifiInit()
{
    WiFi.config(local_ip, gateway, subnet, primaryDNS, secondaryDNS);
    WiFi.begin(ssid, password);

    // Check the status of WiFi connection.
    int tryCount = 0;
    Serial.println("Connecting to WiFi..");
    while (WiFi.status() != WL_CONNECTED && tryCount < 10)
    {
        Serial.print('.');
        tryCount++;
        delay(1000);
    }

    if (WiFi.status() == WL_CONNECTED)
    {
        // If connection successful show IP address in serial monitor
        Serial.print("Connected to WiFi network with IP Address: ");
        Serial.println(WiFi.localIP());
    }
    else
    {
        // If WiFi is not connected then start AP
        Serial.println("Starting ESP32 as softAP...");
        // Configure static IP Address
        if (!WiFi.softAPConfig(local_ip, softap_gateway, subnet)) {
            Serial.println("SoftAP Configuration failed.");
            return;
        }
        // Start softAP
        if (!WiFi.softAP(softap_ssid, softap_password)) {
            Serial.println("SoftAP failed to start.");
            return;
        }
        Serial.print("Access Point IP address: ");
        Serial.println(WiFi.softAPIP());
    }
}

void setup()
{
    Serial.begin(9600); delay(10);
    Serial.println(); Serial.println("Starting...");

    pinMode(relay_heat, OUTPUT);
    pinMode(relay_fan, OUTPUT);
    digitalWrite(relay_heat, relay_heat_status);
    digitalWrite(relay_fan, relay_fan_status);

    EEPROM.begin(512); // fetch the calibration value from eeprom
    unsigned long stabilizingTime = 2000;
    boolean _tare = true;  //set this to false if you don't want tare to be performed in the next step

    dht.begin();
    loadCell.begin();

    loadCell.start(stabilizingTime, _tare);
    if (isnan(hum) || isnan(tem))
    {
        Serial.println(F("Timeout, check DHT22 wiring and pin designations."));
        while (1);
    }
    if (loadCell.getTareTimeoutFlag())
    {
        Serial.println("Timeout, check MCU>HX711 wiring and pin designations.");
        while (1);
    } else
    {
        loadCell.setCalFactor(weightCalValue);
        Serial.println("Startup is complete.");
    }

    attachInterrupt(digitalPinToInterrupt(HX711_dout), dataReadyISR, FALLING);

    /* Network code */
    wifiInit();
}

/* interrupt routune for HX711 */
void dataReadyISR()
{
    if (loadCell.update())
    {
        weight_newDataReady = 1;
    }
}

void loop() {
    hum = dht.readHumidity();
    tem = dht.readTemperature();
    
    if (weight_newDataReady)
    {
        weight = loadCell.getData();
        weight_newDataReady = 0;
    }

    if(millis() >= time_now + oneSecPeriod)
    {
        //serialPlotData();
        wifiRecon();
        time_now = millis();
    }

    // receive comand from serial terminal
    if (Serial.available() > 0)
    {
        char inByte = Serial.read();
        readSerialCMD(inByte);
    }

    // check if last tare operation is complete
    if (loadCell.getTareStatus() == true)
    {
        Serial.println("Tare complete");
    }
}

void wifiRecon()
{
    if ((WiFi.status() != WL_CONNECTED))
    {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
}
}


void changeRelayState(int opt = 3) {
    // LOW = enable ; HIGH = disable
    // opt 0 = heat ; opt 1 = fan ; opt 2 = both ; opt 3 = none
    if (opt == 0 || opt == 2)
    {
        if (relay_heat_status == HIGH)
        {
            digitalWrite(relay_heat, LOW);
            relay_heat_status = LOW;
            Serial.println("Heater toggled off.");
        } else
        {
            digitalWrite(relay_heat, HIGH);
            relay_heat_status = HIGH;
            Serial.println("Heater toggled on.");
        }
    }

    if (opt == 1 || opt == 2)
    {
        if (relay_fan_status == HIGH)
        {
            digitalWrite(relay_fan, LOW);
            relay_fan_status = LOW;
            Serial.println("Fan toggled off.");
        } else
        {
            digitalWrite(relay_fan, HIGH);
            relay_fan_status = HIGH;
            Serial.println("Fan toggled off.");
        }
    }
}

void serialPlotData()
{
    Serial.print(F(">humidity:"));
    Serial.println(hum);
    Serial.print(F(">temperature:"));
    Serial.println(tem);

    Serial.print(">weight:");
    Serial.println(weight);
}

void readSerialCMD(char inByte)
{
    switch (inByte) {
    case 't': // set tare
        loadCell.tareNoDelay();
        break;
    case 'i': // toggle heater
        changeRelayState(0);
        break;
    case 'o': //  toggle fan
        changeRelayState(1);
        break;
    case 'p': // toggle both
        changeRelayState(2);
        break;
    }
}
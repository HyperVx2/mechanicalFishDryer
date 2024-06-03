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
#include <WebServer.h>

/* Webserver code */
// network credentials
const char* ssid = "Alpha 9004";
const char* password = "WarayPassword123";

const char* softap_ssid = "MFD_ESP32";
const char* softap_password = "mfDOOM1234";

// IP Address details for softAP
IPAddress local_ip(192,168,0,225);
IPAddress gateway(192,168,0,254);
IPAddress softap_gateway(192,168,0,225);
IPAddress subnet(255,255,255,0);

WebServer server(80);

/* scheduler code */
const int period = 1000; // in ms
unsigned long time_now = 0;

/* Relay code */
#define relay_heat 33
#define relay_fan 14

bool relay_heat_status = HIGH;
bool relay_fan_status = HIGH;

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
volatile boolean weight_newDataReady;

float weightCalValue = 229.63; // todo: change value
float weight = 0; // weight data

void setup() {
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

    /* Network code */
    WiFi.config(local_ip, gateway, subnet);
    WiFi.begin(ssid, password);

    // Check the status of WiFi connection.
    int tryCount = 0;
    while (WiFi.status() != WL_CONNECTED && tryCount < 10) {
        delay(1000);
        Serial.println("Connecting to WiFi...");
        tryCount++;
    }

    if(WiFi.status() == WL_CONNECTED) {
        // If connection successful show IP address in serial monitor
        Serial.print("Connected to WiFi network with IP Address: ");
        Serial.println(WiFi.localIP());
    } else {
        // If WiFi is not connected then start AP
        Serial.println("Starting ESP32 as softAP...");
        // Configure static IP Address
        if (!WiFi.softAPConfig(local_ip, softap_gateway, subnet)) {
            Serial.println("SoftAP Configuration failed.");
            return;
        }
        // Start softAP
        if(!WiFi.softAP(softap_ssid, softap_password)) {
            Serial.println("SoftAP failed to start.");
            return;
        }
        Serial.print("Access Point IP address: ");
        Serial.println(WiFi.softAPIP());
    }
    
    /* Webserver code */
    server.on("/", handle_OnConnect);
    server.on("/toggleHeat", handle_ToggleHeat);
    server.on("/toggleFan", handle_ToggleFan);
    server.on("/toggleBoth", handle_ToggleBoth);
    server.onNotFound(handle_NotFound);

    server.begin();
    Serial.println("HTTP server started.");
}

/* interrupt routune for HX711 */
void dataReadyISR() {
    if (loadCell.update()) {
        weight_newDataReady = 1;
    }
}

void loop() {
    server.handleClient();

    hum = dht.readHumidity();
    tem = dht.readTemperature();
    
    if (weight_newDataReady) {
        weight = loadCell.getData();
        weight_newDataReady = 0;
    }

    if(millis() >= time_now + period){
        serialPlotData();
        time_now = millis();
    }

    // receive comand from serial terminal
    if (Serial.available() > 0) {
        char inByte = Serial.read();
        readSerialCMD(inByte);
    }

    // check if last tare operation is complete
    if (loadCell.getTareStatus() == true) {
        Serial.println("Tare complete");
    }
}

void changeRelayState(int opt = 3) {
    // LOW = enable ; HIGH = disable
    // opt 0 = heat ; opt 1 = fan ; opt 2 = both ; opt 3 = none
    if (opt == 0 || opt == 2) {
        if (relay_heat_status == HIGH) {
            digitalWrite(relay_heat, LOW);
            relay_heat_status = LOW;
            Serial.println("Heater toggled off.");
        } else {
            digitalWrite(relay_heat, HIGH);
            relay_heat_status = HIGH;
            Serial.println("Heater toggled on.");
        }
    }

    if (opt == 1 || opt == 2) {
        if (relay_fan_status == HIGH) {
            digitalWrite(relay_fan, LOW);
            relay_fan_status = LOW;
            Serial.println("Fan toggled off.");
        } else {
            digitalWrite(relay_fan, HIGH);
            relay_fan_status = HIGH;
            Serial.println("Fan toggled off.");
        }
    }
}

void serialPlotData() {
    Serial.print(F(">humidity:"));
    Serial.println(hum);
    Serial.print(F(">temperature:"));
    Serial.println(tem);

    Serial.print(">weight:");
    Serial.println(weight);
}

void readSerialCMD(char inByte) {
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

/* Webserver functions */
void handle_OnConnect() {
    server.send(200, "text/html", SendHTML(relay_heat_status, relay_fan_status));
}

void handle_ToggleHeat() {
    changeRelayState(0);
    server.send(200, "text/html", SendHTML(relay_heat_status, relay_fan_status));
}

void handle_ToggleFan() {
    changeRelayState(1);
    server.send(200, "text/html", SendHTML(relay_heat_status, relay_fan_status));
}

void handle_ToggleBoth() {
    changeRelayState(2);
    server.send(200, "text/html", SendHTML(relay_heat_status, relay_fan_status));
}

void handle_NotFound() {
    server.send(404, "text/plain", "Not found");
}

String SendHTML(uint8_t heat, uint8_t fan) {
    String ptr = "<!DOCTYPE html> <html>\n";
    ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
    ptr += "<title>Mechanical Fish Dryer Control Page</title>\n";

    ptr +="<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
    ptr +="body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
    ptr +=".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
    ptr +=".button-on {background-color: #3498db;}\n";
    ptr +=".button-on:active {background-color: #2980b9;}\n";
    ptr +=".button-off {background-color: #34495e;}\n";
    ptr +=".button-off:active {background-color: #2c3e50;}\n";
    ptr +="p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
    ptr +="</style>\n";
    ptr += "<body>\n";
    ptr += "<h1>MFD Control Page</h1>\n";

    if(heat)
    {ptr +="<p>Heater Status: ON</p><a class=\"button button-off\" href=\"/toggleHeat\">OFF</a>\n";}
    else
    {ptr +="<p>Heater Status: OFF</p><a class=\"button button-on\" href=\"/toggleHeat\">ON</a>\n";}
    if(fan)
    {ptr +="<p>Fan Status: ON</p><a class=\"button button-off\" href=\"/toggleFan\">OFF</a>\n";}
    else
    {ptr +="<p>Fan Status: OFF</p><a class=\"button button-on\" href=\"/toggleFan\">ON</a>\n";}
    ptr +="<p>Toggle Both</p><a class=\"button button-on\" href=\"/toggleBoth\">TOGGLE</a>\n";
    
    ptr += "</body>\n";
    ptr += "</html>\n";
    return ptr;
}
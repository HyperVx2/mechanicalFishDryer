/*
    Usable ESP32 pins (in order): 14, 27, 26, 25, 33, 32
    Ref: https://lastminuteengineers.com/esp32-pinout-reference

    Assigned pin (in order): RELAY_HEATER, HX711_DT, HX711_SCK, DHT22, RELAY_FANq, DHT11_CONTROL

    Ref 1 (dht22): https://github.com/adafruit/DHT-sensor-library/blob/master/examples/DHTtester/DHTtester.ino
    Ref 2 (hx711): https://github.com/olkal/HX711_ADC/blob/master/examples/Read_1x_load_cell_interrupt_driven/Read_1x_load_cell_interrupt_driven.ino
    Ref 3 (scheduler): https://www.norwegiancreations.com/2017/09/arduino-tutorial-using-millis-instead-of-delay/
    Ref 4 (InfluxDB/MQTT): https://randomnerdtutorials.com/esp8266-and-node-red-with-mqtt/

    Host: chadaPi.local (10.42.0.1)
    Ports: 8086=InfluxDB, 1880=NodeRED, 1883=MQTT
*/

#include <DHT.h>
#include <HX711_ADC.h>
#include <EEPROM.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#define DEVICE "ESP32"

/* WiFi */
// network credentials
const char* ssid = "SMFDS_rPi4";
const char* password = "foobar123";

// softAP credentials
const char* softap_ssid = "SMFDS_ESP32";
const char* softap_password = "foobar123";

// IP Address details
IPAddress local_ip(10,42,0,2);
IPAddress gateway(10,42,0,1);
IPAddress softap_gateway(10,42,0,2);
IPAddress subnet(255,255,255,0);
IPAddress primaryDNS(1,1,1,1);

/* InfluxDB code */
#define INFLUXDB_URL "http://chadapi.local:8086"
#define INFLUXDB_TOKEN "r_imaCxprc_D9T5cGtEYN3xuaBGKB1nwh2F98DsmNOOakny1TZvx8h22Px_1LV8fS2IcAsx6C-FV-5NcOVVB3g=="
#define INFLUXDB_ORG "d969eda8d1eb6600"
#define INFLUXDB_BUCKET "SMFDS_data"

/* MQTT broker code */
// MQTT broker credentials (set to NULL if not required)
const char* MQTT_username = "chada"; 
const char* MQTT_password = "foobar123"; 

// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "chadapi.local";

WiFiClient espClient;
PubSubClient client(espClient);

// Time zone info
#define TZ_INFO "UTC8"

// Declare InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient clientDB(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

// Data point
Point sensorReadings("measurements");

/* scheduler code */
unsigned int oneSecPeriod = 1000; // 1sec reading
unsigned long time_now = 0;

/* Relay code */
#define relay_heat 33
#define relay_fan 14

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
    WiFi.config(local_ip, gateway, subnet, primaryDNS);
    WiFi.begin(ssid, password);

    // Check the status of WiFi connection.
    int tryCount = 0;
    Serial.print("Connecting to WiFi..");
    while (WiFi.status() != WL_CONNECTED && tryCount < 10)
    {
        Serial.print('.');
        tryCount++;
        delay(1000);
        Serial.println(" ");
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

// This function is executed when some device publishes a message to a topic that your ESP32 is subscribed to
void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();

  if(topic=="machine/heater"){
      Serial.print("Changing heater to ");
      if(messageTemp == "on"){
        digitalWrite(relay_heat, LOW);
        Serial.print("ON");
      }
      else if(messageTemp == "off"){
        digitalWrite(relay_heat, HIGH);
        Serial.print("OFF");
      }
  } else if (topic=="machine/fan"){
      Serial.print("Changing fan to ");
      if(messageTemp == "on"){
        digitalWrite(relay_fan, LOW);
        Serial.print("ON");
      }
      else if(messageTemp == "off"){
        digitalWrite(relay_fan, HIGH);
        Serial.print("OFF");
      }
  }
  Serial.println();
}

// This functions reconnects your ESP8266 to your MQTT broker
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP32", MQTT_username, MQTT_password)) {
      Serial.println("connected");  
      // Subscribe or resubscribe to a topic
      client.subscribe("machine/heater");
      client.subscribe("machine/fan");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup()
{
    Serial.begin(9600); delay(10);
    Serial.println(); Serial.println("Starting...");

    /* init relay module */
    pinMode(relay_heat, OUTPUT);
    pinMode(relay_fan, OUTPUT);
    digitalWrite(relay_heat, HIGH);
    digitalWrite(relay_fan, HIGH);

    /* init hx711/dht22 */
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

    /* init wifi */
    wifiInit();

    // Accurate time is necessary for certificate validation and writing in batches
    // For the fastest time sync find NTP servers in your area: https://www.pool.ntp.org/zone/
    // Syncing progress and the time will be printed to Serial.
    timeSync(TZ_INFO, "0.us.pool.ntp.org");

    /* init InfluxDB */
    // Check server connection
    if (clientDB.validateConnection())
    {
          Serial.print("Connected to InfluxDB: ");
          Serial.println(clientDB.getServerUrl());
    } else
    {
          Serial.print("InfluxDB connection failed: ");
          Serial.println(clientDB.getLastErrorMessage());
    }

    /* init MQTT */
    client.setServer(mqtt_server, 1883);
    client.setCallback(callback);
}

/* interrupt routune for HX711 */
void dataReadyISR()
{
    if (loadCell.update())
    {
        weight_newDataReady = 1;
    }
}

void loop()
{
    if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("ESP32", MQTT_username, MQTT_password);

    if(millis() >= time_now + oneSecPeriod)
    { 
        tem = dht.readTemperature();
        hum = dht.readHumidity();

        if (weight_newDataReady)
        {
            weight = loadCell.getData();
            weight_newDataReady = 0;
        }

        //serialPlotData();
        sendToInfluxDB();
        client.publish("machine/temperature", String(tem).c_str());
        client.publish("machine/humidity", String(hum).c_str());
        client.publish("machine/weight", String(weight).c_str());

        wifiRecon();

        time_now = millis();
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

void sendToInfluxDB()
{
    // Add readings as fields to point
    sensorReadings.addField("temperature", tem);
    sensorReadings.addField("humidity", hum);
    sensorReadings.addField("weight", weight);

    // Print what are we exactly writing
    Serial.print("Writing: ");
    Serial.println(clientDB.pointToLineProtocol(sensorReadings));
    
    // Write point into buffer
    clientDB.writePoint(sensorReadings);

    // Clear fields for next usage. Tags remain the same.
    sensorReadings.clearFields();
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
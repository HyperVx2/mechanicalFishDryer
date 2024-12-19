#include "web_cards.h"

extern AsyncWebServer server();

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void mqtt_callback(char* topic, byte* message, unsigned int length) {
  /*Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: "); */
  String messageTemp;

  for (int i = 0; i < length; i++) {
    //Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  //Serial.println();

  if (strcmp(topic, "esp32/sensors/settare") == 0) {
    if (messageTemp == "true") {
      setTareHX711();
    }
  } else if (strcmp(topic, "esp32/heater") == 0) {
    //Serial.print("Changing heater state to ");
    toggleActuator("heater", messageTemp == "true" ? HIGH : LOW);
    //Serial.println(messageTemp);
  } else if (strcmp(topic, "esp32/fan") == 0) {
    //Serial.print("Changing fan state to ");
    toggleActuator("fan", messageTemp == "true" ? HIGH : LOW);
    //Serial.println(messageTemp);
  } else if (strcmp(topic, "esp32/timer/start") == 0) {
    setTimer(messageTemp.toInt());
  } else if (strcmp(topic, "esp32/timer/add") == 0) {
    setTimer(messageTemp.toInt(), true);
  } else if (strcmp(topic, "esp32/timer/reset") == 0) {
    if (messageTemp == "true") {
      resetTimer();
    }
  }
}

boolean mqtt_reconnect() {
  if (client.connect(MQTT_CLIENT_NAME, MQTT_USERNAME, MQTT_PASSWORD)) {
    // Subscribe to a mqtt topics
    client.subscribe("esp32/sensors/settare");
    client.subscribe("esp32/heater");
    client.subscribe("esp32/fan");
    client.subscribe("esp32/timer/start");
    client.subscribe("esp32/timer/add");
    client.subscribe("esp32/timer/reset");
  }

  return client.connected();
}


void beginMQTT() {
    client.setServer(mqtt.c_str(), 1883);
    client.setCallback(mqtt_callback);
}

void loopMQTT() {
  unsigned long remainingTime = getRemainingTime();

  if (!client.connected()) {
    //Serial.println("Reconnecting to MQTT server...");
    mqtt_reconnect();
    /* if () {
      //Serial.println("Reconnected to MQTT server!");
    }  else {
      Serial.println("Reconnection failed!");
    } */
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 1000) {
    lastMsg = now;

    // Create a JSON document
    JSONVar sensor;

    // Add values to JSON
    sensor["tem"] = temperature;
    sensor["hum"] = humidity;
    sensor["wei"] = weight;
    sensor["cur"] = current;
    sensor["vol"] = voltage;

    // Serialize JSON to String
    String jsonString = JSON.stringify(sensor);

    // Publish the JSON string
    client.publish("esp32/sensors", jsonString.c_str());
    client.publish("esp32/timer", String(remainingTime).c_str());
  }
}

void home(AsyncWebServer *server) {
    // Route for root / web page

    server->on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(LittleFS, "/index.html", "text/html");
    });

    server->serveStatic("/", LittleFS, "/");

    server->onNotFound([](AsyncWebServerRequest *request){
        request->send(404, "text/html", "<h1>The file or content was not found.</h1>");
    });

}

// Note that server is a pointer entering serve(),
// it doesn't need to be derefenced again.
void serve(AsyncWebServer *server) {
    // Home Page
    home(server);
}
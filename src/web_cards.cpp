#include "web_cards.h"

extern AsyncWebServer server();

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

String getReadings() {
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
  return jsonString;
}

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

    // Publish the JSON string
    client.publish("esp32/sensors", getReadings().c_str());
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

void cards(AsyncWebServer *server) {
  // Route for various card functions

  server->on("/readings", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", getReadings());
  });

  server->on("/relay", HTTP_GET, [](AsyncWebServerRequest *request){
    JSONVar relay;
    relay["heater"] = (digitalRead(RELAY_HEAT) ? "OFF" : "ON");
    relay["fan"] = (digitalRead(RELAY_FAN) ? "OFF" : "ON");

    request->send(200, "application/json", JSON.stringify(relay));
  });

  // Set tare for weight sensor
  server->on("/tare", HTTP_GET, [](AsyncWebServerRequest *request){
    setTareHX711();
    request->send(LittleFS, "/index.html", String(), false);
  });

  // Control heater, 0 = off, 1 = on
  server->on("/rh0", HTTP_GET, [](AsyncWebServerRequest *request){
    toggleActuator("heater", LOW);    
    request->send(LittleFS, "/index.html", String(), false);
  });

  server->on("/rh1", HTTP_GET, [](AsyncWebServerRequest *request){
    toggleActuator("heater", HIGH);    
    request->send(LittleFS, "/index.html", String(), false);
  });

  // Control fan, 0 = off, 1 = on
  server->on("/rf0", HTTP_GET, [](AsyncWebServerRequest *request){
    toggleActuator("fan", LOW);    
    request->send(LittleFS, "/index.html", String(), false);
  });

  server->on("/rf1", HTTP_GET, [](AsyncWebServerRequest *request){
    toggleActuator("fan", HIGH);    
    request->send(LittleFS, "/index.html", String(), false);
  });

  server->on("/timer", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "application/json", String(getRemainingTime()));
  });

  server->on("/modTimer", HTTP_GET, [](AsyncWebServerRequest *request){
    if (request->hasParam("mode") && request->hasParam("value")) {
        String mode = request->getParam("mode")->value();
        int value = request->getParam("value")->value().toInt();

        if (mode == "add") {
            setTimer(value * 60 * 1000, true);
        } else if (mode == "set") {
            setTimer(value * 60 * 1000, false);
        } else if (mode == "reset") {
            resetTimer();
        } else {
            request->send(400, "text/plain", "Invalid mode");
            return;
        }
        request->send(200, "text/plain", "Timer updated");
    } else {
        request->send(400, "text/plain", "Missing mode or value parameter");
    }
});
}

// Note that server is a pointer entering serve(),
// it doesn't need to be derefenced again.
void serve(AsyncWebServer *server) {
    // Home Page
    home(server);

    cards(server);
}
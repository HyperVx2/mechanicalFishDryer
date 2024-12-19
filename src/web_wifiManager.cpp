#include "web_wifiManager.h"

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncEventSource events("/events");

// Search for parameter in HTTP POST request
const char* PARAM_INPUT_1 = "ssid";
const char* PARAM_INPUT_2 = "pass";
const char* PARAM_INPUT_3 = "ip";
const char* PARAM_INPUT_4 = "gateway";
const char* PARAM_INPUT_5 = "mqtt";

//Variables to save values from HTML form
String ssid, pass, ip, gateway, mqtt;

// File paths to save input values permanently
const char* ssidPath = "/ssid.txt";
const char* passPath = "/pass.txt";
const char* ipPath = "/ip.txt";
const char* gatewayPath = "/gateway.txt";
const char* mqttPath = "/mqtt.txt";

IPAddress localIP;

// Set Gateway IP address
IPAddress localGateway;
IPAddress subnet(255, 255, 0, 0);

boolean restart = false;

// Initialize LittleFS
void initLittleFS() {
  if (!LittleFS.begin(true)) {
    Serial.println("An error has occurred while mounting LittleFS");
  }
  Serial.println("LittleFS mounted successfully");
}

// Read File from LittleFS
String readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\r\n", path);

  File file = fs.open(path);
  if(!file || file.isDirectory()){
    Serial.println("- failed to open file for reading");
    return String();
  }
  
  String fileContent;
  while(file.available()){
    fileContent = file.readStringUntil('\n');
    break;     
  }
  return fileContent;
}

// Write file to LittleFS
void writeFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Writing file: %s\r\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("- failed to open file for writing");
    return;
  }
  if(file.print(message)){
    Serial.println("- file written");
  } else {
    Serial.println("- write failed");
  }
}

// Function to handle writing parameters to files
void handleParameter(const String& name, const String& value, const char* path) {
  Serial.printf("%s set to: %s\n", name.c_str(), value.c_str());
  writeFile(LittleFS, path, value.c_str());
}

// Initialize WiFi
bool initWiFi() {
  if(ssid=="" || ip==""){
    Serial.println("Undefined SSID or IP address.");
    return false;
  }

  WiFi.mode(WIFI_STA);
  localIP.fromString(ip.c_str());
  localGateway.fromString(gateway.c_str());

  if (!WiFi.config(localIP, localGateway, subnet)){
    Serial.println("STA Failed to configure");
    return false;
  }
  WiFi.begin(ssid.c_str(), pass.c_str());

  Serial.println("Connecting to WiFi...");
  unsigned long startAttemptTime = millis();

  while (WiFi.status() != WL_CONNECTED && millis() - startAttemptTime < 10000) {
    //delay(100);
  }

  if(WiFi.status() != WL_CONNECTED) {
    Serial.println("Failed to connect.");
    return false;
  }

  Serial.println(WiFi.localIP());
  return true;
}

void setupSSID(){
  // Web Server Root URL
  server.on("/conn", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(LittleFS, "/wifimanager.html", "text/html"); });

  server.serveStatic("/conn", LittleFS, "/");

  server.on("/conn", HTTP_POST, [](AsyncWebServerRequest *request)
            {
      int params = request->params();
      for(int i=0;i<params;i++){
        const AsyncWebParameter* p = request->getParam(i);
        if(p->isPost()){
          String name = p->name();
          String value = p->value().c_str();
          if (name == PARAM_INPUT_1) {
            ssid = value;
            handleParameter("SSID", ssid, ssidPath);
          } else if (name == PARAM_INPUT_2) {
            pass = value;
            handleParameter("Password", pass, passPath);
          } else if (name == PARAM_INPUT_3) {
            ip = value;
            handleParameter("IP Address", ip, ipPath);
          } else if (name == PARAM_INPUT_4) {
            gateway = value;
            handleParameter("Gateway", gateway, gatewayPath);
          } else if (name == PARAM_INPUT_5) {
            mqtt = value;
            handleParameter("MQTT", mqtt, mqttPath);
          }
        }
      }
      restart = true;
      request->send(200, "text/plain", "Done. ESP will restart, connect to your router and go to IP address: " + ip); });
}

void beginWifiManager() {
  WiFi.disconnect();
  initLittleFS();
  
  // Load values saved in LittleFS
  ssid = readFile(LittleFS, ssidPath);
  pass = readFile(LittleFS, passPath);
  ip = readFile(LittleFS, ipPath);
  gateway = readFile (LittleFS, gatewayPath);
  mqtt = readFile (LittleFS, mqttPath);
  Serial.print("ssid:"); Serial.println(ssid);
  Serial.print("pass:"); Serial.println(pass);
  Serial.print("ip:"); Serial.println(ip);
  Serial.print("gateway:"); Serial.println(gateway);
  Serial.print("mqtt:"); Serial.println(mqtt);

  setupSSID();
  serve(&server);

  // Event Source
  events.onConnect([](AsyncEventSourceClient *client){
    if(client->lastId()){
      Serial.printf("Client reconnected! Last message ID that it got is: %u\n", client->lastId());
    }
    // send event with message "hello!", id current millis
    client->send("hello!", NULL, millis());
  });
  server.addHandler(&events);

  if(initWiFi()) {
    beginMQTT();
    server.begin();
  }
  else {
    // Connect to Wi-Fi network with SSID and password
    Serial.println("Setting AP (Access Point)");
    // NULL sets an open Access Point
    WiFi.softAP(WIFI_SSID, WIFI_PASS);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP address: ");
    Serial.println(IP);

    ssid = WIFI_SSID; ip = IP.toString();
    
    server.begin();
  }
}

void loopWifiManager() {
  if (restart){
    //delay(5000);
    ESP.restart();
  }
  
  loopMQTT();
}
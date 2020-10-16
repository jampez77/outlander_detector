
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <ArduinoJson.h>
#include "My_Helper.h"

//ALL CONFIG CHANGES ARE LOCATED IN My_Helper.h//
//ALL CONFIG CHANGES ARE LOCATED IN My_Helper.h//
//ALL CONFIG CHANGES ARE LOCATED IN My_Helper.h//
//ALL CONFIG CHANGES ARE LOCATED IN My_Helper.h//
//ALL CONFIG CHANGES ARE LOCATED IN My_Helper.h//
//ALL CONFIG CHANGES ARE LOCATED IN My_Helper.h//
//ALL CONFIG CHANGES ARE LOCATED IN My_Helper.h//
//ALL CONFIG CHANGES ARE LOCATED IN My_Helper.h//
//ALL CONFIG CHANGES ARE LOCATED IN My_Helper.h//

const char* prevStatus = "";
int awayCount = 0;

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  // put your setup code here, to run once:
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  client.setBufferSize(1024);

  while(!configDetailsSent){
    if(connectClient()){
      sendConfigDetailsToHA();
    }
  }
  
  //Setup OTA
  {
    ArduinoOTA.setHostname((mqttDeviceClientId + "-" + String(ESP.getChipId())).c_str());

    ArduinoOTA.onStart([]() {
      Serial.println("Start");
    });
    ArduinoOTA.onEnd([]() {
      Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });
    ArduinoOTA.begin();
  }
}

void loop() {
  //OTA client code
  ArduinoOTA.handle();

  //attempt connection to WiFi if we don't have it
  if(!espClient.connected()){
    setup_wifi();
  }
  
  if (client.connected()) {
    client.loop();
    delay(500);
    digitalWrite(ledPin, HIGH);
    startWiFiScan();
  } else {
    connectClient();
  }
}

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  //Set WiFi mode so we don't create an access point.
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    digitalWrite(ledPin, LOW);
  }

  digitalWrite(ledPin, HIGH);
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

boolean connectClient() {
  // Loop until we're connected
  while (!client.connected()) {
    configDetailsSent = false;
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
    Serial.print("Attempting MQTT connection...");
    // Check connection
    if (client.connect(mqttDeviceClientId.c_str(), mqtt_user, mqtt_password, availabilityTopic, 0, true, payloadNotAvailable)) {
      // Make an announcement when connected
      Serial.println("connected");
      client.publish(availabilityTopic, "super online?", true);
      client.publish(availabilityTopic, payloadAvailable, true);
      client.publish(stateTopic, prevStatus, true);

      client.subscribe(commandTopic);
      client.subscribe(availabilityTopic);
      client.subscribe(resetCommandTopic);

      Serial.println("Subscribed to: ");
      Serial.println(commandTopic);
      Serial.println(availabilityTopic);
      Serial.println(resetCommandTopic);
      
      while(!configDetailsSent){
        sendConfigDetailsToHA();
      }
      return true;
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
      return false;
    }
  }
  return true;
}

void callback(char* topic, byte* message, unsigned int length) {

  String messageStr;
  
  for (int i = 0; i < length; i++) {
    messageStr += (char)message[i];
  }

  Serial.println(topic);

  if(String(topic) == availabilityTopic){
      configDetailsSent = false;
      while(!configDetailsSent){
        sendConfigDetailsToHA();
      }
  }

  if(String(topic) == resetCommandTopic && String(messageStr) == "ON"){
    ESP.restart();
  }
}


void startWiFiScan(){
  //start a scan of available WiFi networks
  Serial.println("Scanning WiFi Networks");
  WiFi.scanNetworksAsync(scanResult);
}

void scanResult(int available_networks){
  
  if(available_networks >= 0){
    
    int carSSIDCount = 0;
    
    StaticJsonDocument<512> networks;
    //add each network to the debug log and increment the count for our cars SSID
    for (int network = 0; network < available_networks; network++) {
      String networkSSID = WiFi.SSID(network);
      networks[network] = networkSSID;
      if(networkSSID == carSSID){
        carSSIDCount++;
      }  
    }

    const char* currentStatus = payloadOn;
    if(carSSIDCount == 0){
      currentStatus = payloadOff;
    }

    //account for any false 'away' negatives by forcing a check before sending that status out.
    if(currentStatus == "away" && awayCount < maxNumAwayTries){
      awayCount++;
    }else{
      //reset away count back to zero after maxNumAwayTries.
      awayCount = 0;
    }

     //build JSON of all networks to send to debug log.
    char json[512];
    
    //we only need to update MQTT server is the status has changed.
    if(prevStatus != currentStatus){
      bool shouldUpdate = true;
      if(currentStatus == "away" && awayCount < maxNumAwayTries){
        shouldUpdate = false;
      }

      //only update if status is 'home' or enough 'away' statuses have been called
      if(shouldUpdate){
        client.publish(stateTopic, currentStatus, true); 
        //update previous status.
        prevStatus = currentStatus;
      }
    }
    //remove scan results from memory.
    WiFi.scanDelete();
   }
}

void sendConfigDetailsToHA(){
  //Send cover entity details to home assistant on initial connection
    //for auto discovery

    DynamicJsonDocument mqttDevConfig(225);
    mqttDevConfig["name"] = mqttDeviceName;
    mqttDevConfig["mf"] = manufacturer;
    mqttDevConfig["mdl"] = model;
    mqttDevConfig["sw"] = softwareVersion;
    mqttDevConfig["ids"][0] = mqttDeviceClientId;
    mqttDevConfig["ids"][1] = mqttResetDeviceClientId;

    DynamicJsonDocument mqttConfig(540);
    mqttConfig["name"] = mqttDeviceName;
    mqttConfig["dev_cla"] = mqttDeviceClass;
    mqttConfig["stat_t"] = stateTopic;
    mqttConfig["pl_on"] = payloadOn;
    mqttConfig["pl_off"] = payloadOff;
    mqttConfig["qos"] = 1;
    mqttConfig["avty_t"] = availabilityTopic;
    mqttConfig["uniq_id"] = mqttDeviceClientId;
    mqttConfig["dev"] = mqttDevConfig;

    char json[540];
    serializeJsonPretty(mqttConfig, json);
    client.publish(configTopic, json, false);

    DynamicJsonDocument mqttResetConfig(505);
    mqttResetConfig["name"] = mqttResetDeviceName;
    mqttResetConfig["ic"] = "mdi:lock-reset";
    mqttResetConfig["cmd_t"] = resetCommandTopic;
    mqttResetConfig["stat_t"] = resetStateTopic;
    mqttResetConfig["avty_t"] = availabilityTopic;
    mqttResetConfig["uniq_id"] = mqttResetDeviceClientId;
    mqttResetConfig["dev"] = mqttDevConfig;

    char resetJson[505];
    serializeJsonPretty(mqttResetConfig, resetJson);
    client.publish(resetConfigTopic, resetJson, false);
    configDetailsSent = true;
}

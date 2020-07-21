
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
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
  
  randomSeed(micros());
  
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    digitalWrite(ledPin, HIGH);
    delay(100);
    digitalWrite(ledPin, LOW);
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    String clientId = "CarDetector-" + String(random(0xffff), HEX);
    if (client.connect(clientId.c_str(), mqtt_user, mqtt_password)) {
      Serial.println("connected");
      client.publish(outputTopic, "connected", true); 
      client.subscribe(debugTopic);
      Serial.println("Subscribed to: ");
      Serial.println(debugTopic);
      startWiFiScan();
      digitalWrite(ledPin, HIGH);
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  // put your setup code here, to run once:
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  while (!client.connected()) {
    reconnect();
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  delay(500);
  startWiFiScan();
}

void startWiFiScan(){
  //start a scan of available WiFi networks
  WiFi.scanNetworksAsync(scanResult);
}


void callback(char* callbackTopic, byte* message, unsigned int length) {

  String messageStr;
  
  for (int i = 0; i < length; i++) {
    messageStr += (char)message[i];
  }

  Serial.println(callbackTopic);

  if(String(callbackTopic) == debugTopic && String(messageStr) == "true" && isDebuggable == false){
    isDebuggable = true;
    client.publish(outputTopic, "debugging on", true); 
  }else if(String(callbackTopic) == debugTopic && String(messageStr) == "false" && isDebuggable == true){
    isDebuggable = false;
    client.publish(outputTopic, "debugging off", true); 
  }

  if(String(callbackTopic) == debugTopic && String(messageStr) == "reset"){
    client.publish(outputTopic, "resetting", true); 
    Serial.println("Resetting..");
    delay(100);
    ESP.restart();
  }

  if(String(callbackTopic) == availabilityTopic){
    //tell home assistant we are online if we get an availability request
    client.publish(callbackTopic, "online", true); 
    delay(100);
    //update home assistant with last known status (if we have one!)
    if(String(prevStatus).length() > 0){
      //sending prevStatus will stop home assistant from defaultng to away
      //after a restart.
      client.publish(outputTopic, prevStatus, true); 
    }
    
  }
}

void scanResult(int available_networks){
  if(isDebuggable){
    Serial.print("Available Networks: ");
    Serial.println(available_networks);
  }
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

    char* currentStatus = "home";
    if(carSSIDCount == 0){
      currentStatus = "away";
    }

    //account for any false 'away' negatives by forcing a check before sending that status out.
    if(currentStatus == "away" && awayCount < maxNumAwayTries){
      awayCount++;
      if(isDebuggable){
        Serial.print("Away count: ");
        Serial.println(awayCount);
      }
    }else{
      //reset away count back to zero after maxNumAwayTries.
      awayCount = 0;
    }

     //build JSON of all networks to send to debug log.
    char json[512];
    
    if(isDebuggable){
      Serial.println(" ");
      serializeJsonPretty(networks, Serial);
      serializeJsonPretty(networks, json);
      client.publish(outputTopic, json, true); 
      Serial.println(" ");
    }
    
    //we only need to update MQTT server is the status has changed.
    if(prevStatus != currentStatus){
      bool shouldUpdate = true;
      if(currentStatus == "away" && awayCount < maxNumAwayTries){
        shouldUpdate = false;
      }

      //only update if status is 'home' or enough 'away' statuses have been called
      if(shouldUpdate){
        client.publish(outputTopic, currentStatus, true); 
        //update previous status.
        prevStatus = currentStatus;
      }
    }
    //remove scan results from memory.
    WiFi.scanDelete();
   }
}

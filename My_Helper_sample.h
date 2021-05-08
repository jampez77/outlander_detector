#ifndef MY_HELPER_H
#define MY_HELPER_H

//IN ORDER TO USE THIS FILE
//UPDATE IT WITH YOUR OWN DETAILS 
//AND RENAME IT TO My_Helper.h

//The WiFi details for you own network
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

//The server and login details for your MQTT broker
const char* mqtt_user = "YOUR_MQTT_USERNAME";
const char* mqtt_password = "YOUR_MQTT_PASSWORD";
const char* mqtt_server = "YOUR_MQTT_SERVER_ADDRESS";

//The topic names to send to your MQTT broker. You don't have to change these if you don't want to.
const char* stateTopic = "homeassistant/binary_sensor/garage/carHomeSensor/state";
const char* debugTopic = "homeassistant/binary_sensor/garage/carHomeSensor/debug";
const char* commandTopic = "homeassistant/binary_sensor/garage/carHomeSensor/set";
const char* configTopic = "homeassistant/binary_sensor/garage/carHomeSensor/config";
const char* availabilityTopic = "homeassistant/binary_sensor/garage/carHomeSensor/availability";
String mqttDeviceClientId = "CHS";
const char* mqttDeviceName = "Car Home Sensor";
const char* mqttDeviceClass = "presence";

const char* resetCommandTopic = "homeassistant/switch/garage/carHomeSensor/set";
const char* resetStateTopic = "homeassistant/switch/garage/carHomeSensor/state";
const char* resetConfigTopic = "homeassistant/switch/garage/carHomeSensor/config";
String mqttResetDeviceClientId = "RstCHS";
const char* mqttResetDeviceName = "Reset";

const char* softwareVersion = "1.6";
const char* manufacturer = "NandPez";
const char* model = "Outlander Detector";

//The maximum number of 'away' finds before we decide to set the status as 'away'
//Increase this number if you are getting a lot of false triggers for 'away'
int maxNumAwayTries = 5;

//The WiFi SSID of your outlander I.E REMOTE******
const char* carSSID = "YOUR_CARS_SSID";
const int ledPin = D4;
boolean configDetailsSent = false;

const char* payloadAvailable = "online";
const char* payloadNotAvailable = "offline";
const char* payloadOn = "home";
const char* payloadOff = "away";

#endif

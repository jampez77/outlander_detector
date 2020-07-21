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
const char* outputTopic = "car/home";
const char* debugTopic = "car/home/debug";
const char* availabilityTopic = "car/home/available";

//Set this to true if you want to send feedback of all WiFi network found, back to your MQTT broker.
bool isDebuggable = false;

//The maximum number of 'away' finds before we decide to set the status as 'away'
//Increase this number if you are getting a lot of false triggers for 'away'
int maxNumAwayTries = 5;

//The WiFi SSID of your outlander I.E REMOTE******
const char* carSSID = "YOUR_CARS_SSID";
const int ledPin = D4;

#endif

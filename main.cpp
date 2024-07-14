/*
 * Example
 *
 * If you encounter any issues:
 * - check the readme.md at https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md
 * - ensure all dependent libraries are installed
 * - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#arduinoide
 * - see https://github.com/sinricpro/esp8266-esp32-sdk/blob/master/README.md#dependencies
 * - open serial monitor and check whats happening
 * - check full user documentation at https://sinricpro.github.io/esp8266-esp32-sdk
 * - visit https://github.com/sinricpro/esp8266-esp32-sdk/issues and check for existing issues or open a new one
 */

 // Custom devices requires SinricPro ESP8266/ESP32 SDK 2.9.6 or later

// Uncomment the following line to enable serial debug output
//#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG
  #define DEBUG_ESP_PORT Serial
  #define NODEBUG_WEBSOCKETS
  #define NDEBUG
#endif

#include <Arduino.h>
#ifdef ESP8266
  #include <ESP8266WiFi.h>
#endif
#ifdef ESP32
  #include <WiFi.h>
#endif

#include <SinricPro.h>
#include "WATERLEVELSENSOR.h"

#define APP_KEY    "****************************************"
#define APP_SECRET "***************************************"// blurred for security purposes lol:)
#define DEVICE_ID  "******************************************"

#define SSID       "**********"//insert wifi name and password here respectivley
#define PASS       "***********"

#define BAUD_RATE  115200
#define EVENT_WAIT_TIME   10000 // send event every 60 seconds
const int trigPin = 25;
  const int echoPin = 33;
const int MIN = 0;
const int MAX = 0.882;
  const int EMPTY_TANK_HEIGHT = 2; // Height when the tank is empty
const int FULL_TANK_HEIGHT  =  15.5; // Height when the tank is full
long duration;
float distanceInCm; 
int waterLevelAsPer;

float liters;
float L;
int vol;
int lastWaterLevelAsPer;
float lastDistanceInCm;

WATERLEVELSENSOR &wATERLEVELSENSOR = SinricPro[DEVICE_ID];

/*************
 * Variables *
 ***********************************************
 * Global variables to store the device states *
 ***********************************************/

// RangeController
std::map<String, int> globalRangeValues;

/*************
 * Callbacks *
 *************/

// RangeController
bool onRangeValue(const String &deviceId, const String& instance, int &rangeValue) {
  Serial.printf("[Device: %s]: Value for \"%s\" changed to %d\r\n", deviceId.c_str(), instance.c_str(), rangeValue);
  globalRangeValues[instance] = rangeValue;
  return true;
}

bool onAdjustRangeValue(const String &deviceId, const String& instance, int &valueDelta) {
  globalRangeValues[instance] += valueDelta;
  Serial.printf("[Device: %s]: Value for \"%s\" changed about %d to %d\r\n", deviceId.c_str(), instance.c_str(), valueDelta, globalRangeValues[instance]);
  globalRangeValues[instance] = valueDelta;
  return true;
}

/**********
 * Events *
 *************************************************
 * Examples how to update the server status when *
 * you physically interact with your device or a *
 * sensor reading changes.                       *
 *************************************************/

// RangeController
void updateRangeValue( int value) {
  wATERLEVELSENSOR.sendRangeValueEvent("rangeInstance1", value);}

  void updateRangeValue2( int value) {
  wATERLEVELSENSOR.sendRangeValueEvent("rangeInstance2", value);
}

// PushNotificationController
void sendPushNotification(String notification) {
  wATERLEVELSENSOR.sendPushNotification(notification);
}

/********* 
 * Setup *
 *********/

void setupSinricPro() {

  // RangeController
  wATERLEVELSENSOR.onRangeValue("rangeInstance1", onRangeValue);
  wATERLEVELSENSOR.onAdjustRangeValue("rangeInstance1", onAdjustRangeValue);
  wATERLEVELSENSOR.onRangeValue("rangeInstance2", onRangeValue);
  wATERLEVELSENSOR.onAdjustRangeValue("rangeInstance2", onAdjustRangeValue);


  SinricPro.onConnected([]{ Serial.printf("[SinricPro]: Connected\r\n"); });
  SinricPro.onDisconnected([]{ Serial.printf("[SinricPro]: Disconnected\r\n"); });
  SinricPro.begin(APP_KEY, APP_SECRET);
};

void setupWiFi() {
  #if defined(ESP8266)
    WiFi.setSleepMode(WIFI_NONE_SLEEP); 
    WiFi.setAutoReconnect(true);
  #elif defined(ESP32)
    WiFi.setSleep(false); 
    WiFi.setAutoReconnect(true);
  #endif

  WiFi.begin(SSID, PASS);
  Serial.printf("[WiFi]: Connecting to %s", SSID);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.printf(".");
    delay(250);
  }
  Serial.printf("connected\r\n");
}

void handleSensor() {
   if (SinricPro.isConnected() == false) {
   
  }

}
  static unsigned long last_millis;
  unsigned long        current_millis = millis();
  if (last_millis && current_millis - last_millis < 1000) return; // Wait untill 1 min
  last_millis = current_millis;

  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  duration = pulseIn(echoPin, HIGH);   
  distanceInCm = 18 - (duration/29 / 2);
  
  if(distanceInCm <= 0) { 
    Serial.printf("Invalid reading: %d..\r\n", distanceInCm); 
    return;
  }
  
 

  if(distanceInCm >= 3) { 
    
   sendPushNotification("TANK IS FULL!"); 
    return;
  }
  
  
  vol = 49 * (distanceInCm);
   L = vol;
   
   lastDistanceInCm = distanceInCm;
  waterLevelAsPer = map((int)distanceInCm ,EMPTY_TANK_HEIGHT, FULL_TANK_HEIGHT, 0, 100); 
  waterLevelAsPer = constrain(waterLevelAsPer, 1, 100);
  
  Serial.printf("Distance (cm): %f. %d%%\r\n", distanceInCm, waterLevelAsPer,duration); 
  
  /* Update water level on server */  
  updateRangeValue(waterLevelAsPer);
  updateRangeValue2(L);  

  /* Send a push notification if the water level is too low! */
  if(waterLevelAsPer <= 15) { 
    sendPushNotification("Water level is too low!.....Pump Water Now");
    } 
}
  

 void PumpRelay(){
   if(waterLevelAsPer <= 15) { 
    sendPushNotification("Water level is too low!.....Pump Water Now");
    } 
  while(waterLevelAsPer <= 15) { 
    sendPushNotification("pump is on!");
   digitalWrite(2,HIGH);
    delay(500);
    digitalWrite(2,LOW);
    delay(500);
  } 
 }
void setupSensor() {
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
}

void setup() {
  pinMode(2,OUTPUT);
   Serial.begin(BAUD_RATE);
  setupSensor();
  setupWiFi();
  setupSinricPro();
}

/********
 * Loop *
 ********/

void loop() {
PumpRelay();
handleSensor();
SinricPro.handle();
}

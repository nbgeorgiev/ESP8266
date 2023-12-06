#include <Wire.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include "SdsDustSensor.h"
#include "wifi_credentials_consts.h"
#define WARM_UP_INTERVAL 3e4 // 3e4 ms is 30 seconds
#define SLEEP_INTERVAL 3e5 // 3e5 ms is 5 minutes - delay() takes milliseconds
#define DEBUG 1 // debug = 1 -> enable

// Serial Comunication with SDS011 sensor
int rxPin = D1;
int txPin = D2;
SdsDustSensor sds(rxPin, txPin);

// Time intervals for the SDS011 sensor
const int warmUpInterval = WARM_UP_INTERVAL; // Waking up of SDS011 - 30 sec
unsigned long startMillis;  //some global variables available anywhere in the program
unsigned long currentMillis;
const unsigned long sleepInterval = SLEEP_INTERVAL;  // Sleeping time of SDS011 - 5 min 300000 ms

// Wi-Fi settings
WiFiClient client;
const char* ssid = WIFI_SSID;
const char* password = PASSWORD;
String newHostname = HOST_NAME;

// ThingsSpeak settings
unsigned long myChannelNumber = CHANNEL_NUMBER;
const char* myWriteAPIKey = WRITE_API_KEY;

// Variables for sensor readings
float pm25;
float pm10;

// Initialization for variables aqi25 and aqi10, AQI (Air Quality Index) based on respective ug/m^3 raw values
int aqi25 = 0;
int aqi10 = 0;

String aqiCategory25 = "";
String aqiCategory10 = "";

// Functions prototypes
void setup_wifi(); // WiFi connection settings
void aqiCalc(); //AQI index calculation function 
void thingSpeak(); //Pushing data to Thingspeak.com
void printValues(); //Printing values into the serial monitor for debugging purposes
//void updateOTA(); //Update over WiFi and WiFi connection settings

// Initilization of SDS011 sensor
void initSDS011(){
    sds.begin();
    ReportingModeResult pm = sds.setQueryReportingMode();
    Serial.println(sds.queryFirmwareVersion().toString()); // Prints firmware version
    Serial.println(sds.setQueryReportingMode().toString()); // Prints reporting mode
}

// Getting data from SDS011
void getDataSDS011() {
    sds.wakeup();
    delay(warmUpInterval); // warm up for 30 seconds

    PmResult pm = sds.queryPm();
    if (pm.isOk()) {
        pm25 = pm.pm25;
        pm10 = pm.pm10;
    } else {
        Serial.print("Could not read values from sensor, reason: ");
        Serial.println(pm.statusToString());
    }

    WorkingStateResult state = sds.sleep();  //Think to fix how not to get stuck!

    if (state.isWorking()) {
        Serial.println("Problem with sleeping the sensor.");
    } else {
        Serial.println("Sensor is sleeping");
    }
}

void setup() {
    Serial.begin(115200);
    setup_wifi();

    // Begin OTA
    ArduinoOTA.begin();
    //updateOTA(); //WiFi connection and OTA update code

    // Sensors initialization functions
    initSDS011();

    // Initial start time
    startMillis = millis();
}

void loop() {
    ArduinoOTA.handle(); // Handle OTA
    currentMillis = millis();
    if (currentMillis - startMillis >= sleepInterval)
    {
        getDataSDS011();
        delay(100);
        aqiCalc();
        delay(100);
        printValues();
        delay(100);
        thingSpeak();
        startMillis = currentMillis;
    }
}

#include "SdsDustSensor.h"
#include "wifi_credentials_consts.h"
#include "OTA.h"
#define WARM_UP_INTERVAL 3e4 // 3e4 ms is 30 seconds
#define SLEEP_INTERVAL 3e5 // 3e5 ms is 5 minutes
#define DEBUG 0 // debug = 1 -> enable

// Serial Comunication with SDS011 sensor
int rxPin = D1;
int txPin = D2;
SdsDustSensor sds(rxPin, txPin);

// Time intervals for the SDS011 sensor
const int warmUpInterval = WARM_UP_INTERVAL; // Warm up of SDS011 for 30 sec before taking measurement
unsigned long startMillis;
unsigned long currentMillis;
const unsigned long sleepInterval = SLEEP_INTERVAL;  // Sleeping time of SDS011 - 5 min 300000 ms

// Wi-Fi settings
WiFiClient client;
const char* ssid = WIFI_SSID;
const char* password = PASSWORD;
const char* newHostname = HOST_NAME;

// ThingsSpeak settings
unsigned long myChannelNumber = CHANNEL_NUMBER;
const char* myWriteAPIKey = WRITE_API_KEY;

// Variables for sensor readings
float pm25;
float pm10;

// Initialization for variables aqi25 and aqi10, AQI (Air Quality Index) based on respective ug/m^3 raw values
int aqi25 = 0;
int aqi10 = 0;

// Functions prototypes
void setupOTA(const char*, const char*, const char*); // Update Over The Air and WiFi settings
void aqi(float, float, int*, int*); // AQI index calculation function 
void thingSpeak(); // Pushing data to Thingspeak.com
void printValues(); // Printing values into the serial monitor for debugging purposes

// Initilization of SDS011 sensor
void initSDS011(){
    sds.begin();
    ReportingModeResult pm = sds.setQueryReportingMode(); // Set query reporting mode
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

    // Arduino OTA settings and WiFi connection
    setupOTA(newHostname, WIFI_SSID, PASSWORD);
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
        aqi(pm25, pm10, &aqi25, &aqi10);
        delay(100);
        printValues();
        delay(100);
        thingSpeak();
        startMillis = currentMillis;
    }
}

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include "wifi_credentials.h"
#include <ThingSpeak.h>
#define sensor_address 0x76 // Adafruit devices use 0x77 all others use 0x76

//---------- I2C mode and BME object
Adafruit_BME280 bme;

//--------- Wi-Fi settings
WiFiClient  client;
const char* ssid = WIFISSID;
const char* password = PASSWORD;
String newHostname = "Battery_Weather_Station";

//--------ThingsSpeak channel settings
unsigned long int myChannelNumber = 123456789;
const char* myWriteAPIKey = "";

// Variables for sensor readings
float tempC;
float humidity;
float pressureAbs;

// Activation of printValues() function for testing/debugging into the Serial Monitor
bool debug = 0; //debug = 1 -> enable 

// Initialization of BME280 sensor
void initBME280() { 
    if (!bme.begin(sensor_address)) {
    Serial.println("Could not find a valid BME280 sensor, check wiring!");
    while (1);
    }
}

// Getting data from BME280
void getDataBME280() {
  tempC = bme.readTemperature();
  pressureAbs = (bme.readPressure()/ 100.0F);
  humidity = bme.readHumidity(); 
     
}

// Printing the data into the Serial Monitor
void printValues() {
  if (debug) {
   //Data from BME280  
   Serial.println("Data from BME280 Sensor:");
   Serial.printf("Temperature = %.1f ºC \n", tempC);
   Serial.printf("Pressure = %.1f hPa \n", pressureAbs);
   Serial.printf("Humidity = %.1f %% \n", humidity);   
  }
}

void connectWiFi() {

  // Connect to Wifi.
  Serial.println("WiFi Status:");
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // WiFi fix: https://github.com/esp8266/Arduino/issues/2186
  WiFi.persistent(false);
  WiFi.mode(WIFI_OFF);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long wifiConnectStart = millis();

  while (WiFi.status() != WL_CONNECTED) {
    // Check to see if
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("Failed to connect to WiFi. Please verify credentials: ");
      delay(10000);
    }

    delay(500);
    Serial.print(".");
    // Only try for 5 seconds.
    if (millis() - wifiConnectStart > 15000) {
      Serial.println("Failed to connect to WiFi");
      return;
    }
 }
}

// Pushing data to Thingspeak.com
void thingSpeak() {
  // set the fields with the values
    ThingSpeak.setField(1, tempC);
    ThingSpeak.setField(2, humidity);
    ThingSpeak.setField(3, pressureAbs);
            
    // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
    // pieces of information in a channel.  Here, we write to field 1.
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    if(x == 200){
      Serial.println("Channel updated successfully into Thingspeak.");
      Serial.println("============================================"); 
    }
    else{
      Serial.println("Problem updating channel. HTTP error code " + String(x));
    }
}

// Function for sleeping the BME280 sensor
void BME280_Sleep(int device_address) {
  // BME280 Register 0xF4 (control measurement register) sets the device mode, specifically bits 1,0
  // The bit positions are called 'mode[1:0]'. See datasheet Table 25 and Paragraph 3.3 for more detail.
  // Mode[1:0]  Mode
  //    00      'Sleep'  mode  00 - 0b00000000
  //  01 / 10   'Forced' mode, use either '01' or '10'  10 = 0b00001010  0b00100111  0x00100100 to set to sleep, and 0x00100101 
  //    11      'Normal' mode
  Serial.println("BME280 to Sleep mode...");
  Wire.beginTransmission(device_address);
  Wire.write((uint8_t)0xF4);       // Select Control Measurement Register
  Wire.write((uint8_t)0b00100100); // Send '10' for 'Forced' mode 
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  Serial.setTimeout(2000);

  // Wait for serial to initialize.
  while (!Serial) { }

  Serial.println("Device Started");
  Serial.println("-------------------------------------");
  Serial.println("Running Deep Sleep Firmware!");
  Serial.println("-------------------------------------");

  // Functions sequences to be run
  initBME280(); //BME280 sensor check
  getDataBME280();
  printValues();
  ThingSpeak.begin(client); // Initialize ThingSpeak  
  connectWiFi();
  thingSpeak();
  BME280_Sleep(sensor_address);  
  Serial.println("Going into deep sleep for 3 minutes!");
  ESP.deepSleep(3e8); // 30e7 is 3 minutes

}

void loop() {
}

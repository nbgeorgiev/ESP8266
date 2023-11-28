#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>
#include <ThingSpeak.h>
#include "wifi_credentials_consts.h"
#define sensor_address 0x76 // Adafruit devices use 0x77 all others use 0x76
#define SLEEP_TIME 3e8 // 3e8 uS is 5 minutes
#define DEBUG 0 //debug = 1 -> enable 
#define CAL_BVOLT 0.077
#define SEA_LEVEL_PRESSURE_CAL 1.0

// BME object
Adafruit_BME280 bme;

// Analog input pin for battery voltage measurment
int analogInPin  = A0;

// Wi-Fi settings
WiFiClient  client;
const char* ssid = WIFI_SSID;
const char* password = PASSWORD;
String newHostname = HOST_NAME;

// ThingsSpeak channel settings
unsigned long int myChannelNumber = CHANNEL_NUMBER;
const char* myWriteAPIKey = WRITE_API_KEY;

// Variables for sensor readings
float tempC, humidity, pressureAbs, pressureSea;
int altitude = 557; // Altitude of the station location in meters
float b = 17.62;
float c = 243.12;
float gamma_var, dewpoint, humidityAbs;

// Battery level variables
int sensorValue;
float voltage;
float calibration = CAL_BVOLT; // Accuracy calibration of voltage measurment through out analog pin
int bat_percentage;
float pressureSea_cal = SEA_LEVEL_PRESSURE_CAL;

// Error codes
int error = 0;

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

void connectWiFi() {
  // Connect to Wifi.
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  WiFi.hostname(newHostname);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long wifiConnectStart = millis();

  while (WiFi.status() != WL_CONNECTED) {
    // Check to see if
    if (WiFi.status() == WL_CONNECT_FAILED) {
      Serial.println("Failed to connect to WiFi. Please verify credentials!");
      delay(1000);
    }

    delay(500);
    Serial.print(".");
    
    // Only try for 20 seconds.
    if (millis() - wifiConnectStart > 20000) {
      Serial.println();
      Serial.println("Failed to connect to WiFi");
      ESP.deepSleep(SLEEP_TIME); // Going to deep sleep if there is not WiFi network available at the moment
    }
 }
 Serial.println();
}

// Initialization of BME280 sensor and sending error if it is not present
void initBME280() { 
    unsigned long init_sensor = millis();

    if (!bme.begin(sensor_address)) {
      Serial.println("Could not find a valid BME280 sensor, check wiring!");
      BME280_Sleep(sensor_address);  //Sleeping the sensor (don't know if it is needed)
      error = 1; // No sensor detected
      ThingSpeak.begin(client); // Initialize ThingSpeak
      connectWiFi(); // Connect to WiFi to transming the error for not operational sensor
      ThingSpeak.setField(7, error); // Pushing error to the error field in ThingSpeak
      
      int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
      if(x == 200){
        Serial.println("Error pushed to Thingspeak!");
        Serial.println("============================================"); 
      }
      else{
        Serial.println("Problem updating channel. HTTP error code " + String(x));
      }
      Serial.println("Going into deep sleep for 5 minutes!");
      ESP.deepSleep(SLEEP_TIME); // 30e8 is 5 minutes
    }
}

// Getting data from BME280
void getDataBME280() {
  tempC = bme.readTemperature();
  pressureAbs = (bme.readPressure()/ 100.0F);
  humidity = bme.readHumidity();

  // Calculate reltive pressure from absolute  
  pressureSea = (((pressureAbs)/pow((1-((float)(altitude))/44330), 5.255))) - pressureSea_cal; // Calculating the sea level pressure (Relative pressure)

  // Calculate dewpoint
  gamma_var = (b*tempC) / (c + tempC) + log(humidity/100.0);
  dewpoint = (c * gamma_var) / (b - gamma_var);

  // Calculate absolute humidity from relative
  humidityAbs = (6.112*exp((b*tempC)/(c + tempC))*humidity*2.1674) / (273.15 + tempC);
}

// Map function for battery level in %
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Battery volatge and level calculation function
void batteryLevel() {
  sensorValue = analogRead(analogInPin); // 0 - 1023 return value
  voltage = ((((float)sensorValue *4.2) / 1024) - calibration); // Since max. battery voltage when charges is 4.2V
  if (voltage < 0){
    voltage = 0;
  }
  bat_percentage = mapfloat(voltage, 3.3, 4.2, 0, 100); //2.8V as Battery Cut off Voltage & 4.2V as Maximum Voltage

  if (bat_percentage >= 100)
  {
    bat_percentage = 100;
  }

  if (bat_percentage <= 0)
  {
    bat_percentage = 1;
  }
}

// Printing the data into the Serial Monitor
void printValues() {
  if (DEBUG) {
   //Data from BME280  
   Serial.println("Data from BME280 Sensor:");
   Serial.printf("Temperature = %.1f ºC \n", tempC);
   Serial.printf("Abs. Pressure = %.1f hPa \n", pressureAbs);
   Serial.printf("Sea level Pressure = %.1f hPa \n", pressureSea);
   Serial.printf("Humidity = %.1f %% \n", humidity);
   Serial.printf("Abs. Humidity = %.1f g/m^3 \n", humidityAbs);  
   Serial.printf("Dew point = %.1f ºC \n", dewpoint);
   Serial.printf("Battery health: \n");
   Serial.printf("Output Voltage = %.3f V \n", voltage);
   Serial.printf("ADC Reading = %d\n", sensorValue);
   Serial.printf("Battery Percentage = %d \n", bat_percentage); 
  }
}

// Pushing data to Thingspeak.com
void thingSpeak() {
    // Set the fields with the values
    ThingSpeak.setField(1, tempC);
    ThingSpeak.setField(2, humidity);
    ThingSpeak.setField(3, pressureAbs);
    ThingSpeak.setField(4, humidityAbs);
    ThingSpeak.setField(5, pressureSea);
    ThingSpeak.setField(6, dewpoint);
    ThingSpeak.setField(8, voltage);
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

void setup() {
  if (DEBUG) {
    Serial.begin(115200);
    Serial.setTimeout(2000);
    // Wait for serial to initialize.
    while (!Serial) { }
    delay(100);
    Serial.println("Device Started");
    Serial.println("-------------------------------------");
    Serial.println("Running Deep Sleep Firmware!");
    Serial.println("-------------------------------------");
  }

  // Functions sequences to be run
  initBME280();
  getDataBME280();
  BME280_Sleep(sensor_address);
  batteryLevel();
  printValues();
  ThingSpeak.begin(client); // Initialize ThingSpeak  
  connectWiFi();
  thingSpeak();
  Serial.println("Going into deep sleep for 5 minutes!");
  ESP.deepSleep(SLEEP_TIME); 
}

void loop() {
}

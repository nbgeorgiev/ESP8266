#include <ESP8266WiFi.h>
#include <ArduinoOTA.h>
#include <Adafruit_BME280.h>
#include <Adafruit_Sensor.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <math.h>
#include <ThingSpeak.h>
#include "wifi_credentials_consts.h"

// Firmware version
#define FIRMWARE_VERSION "v1.0.0"

// Pin definitions
const int ledPin = LED_BUILTIN; // Built-in LED on NodeMCU 0.9 (GPIO2)

// BME280 setup (I2C)
Adafruit_BME280 bme; // I2C interface (D1 = SCL, D2 = SDA)
#define ALTITUDE 210.0 // Altitude in meters (set to 210m for Sv)

// NTP Client setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3 * 3600, 3600000); // UTC+3 (EEST), update every 1 hour

// Function to calculate absolute humidity (g/m³)
float calculateAbsoluteHumidity(float temperature, float humidity) {
  // Validate inputs
  if (isnan(temperature) || isnan(humidity) || humidity < 0 || humidity > 100 || temperature < -40 || temperature > 80) {
    Serial.printf("Invalid input: temperature=%.2f °C, humidity=%.2f %%\n", temperature, humidity);
    return NAN; // Return NaN for invalid inputs
  }
  // Saturation vapor pressure (Tetens formula)
  float svp = 6.1078 * pow(10.0, (7.5 * temperature) / (temperature + 237.3));
  // Actual vapor pressure
  float vp = (humidity / 100.0) * svp;
  // Absolute humidity (g/m³)
  float ah = (216.7 * vp) / (temperature + 273.15); // Correct formula for g/m³
  return ah;
}

// Function to calculate sea-level pressure (hPa)
float calculateSeaLevelPressure(float pressure, float temperature, float altitude) {
  // Validate inputs
  if (isnan(pressure) || isnan(temperature) || pressure <= 0 || temperature < -40 || temperature > 80) {
    Serial.printf("Invalid input: pressure=%.2f hPa, temperature=%.2f °C\n", pressure, temperature);
    return NAN;
  }
  // Barometric formula for sea-level pressure
  float tempKelvin = temperature + 273.15;
  float exponent = -5.257;
  float lapseRate = 0.0065; // Temperature lapse rate (K/m)
  float slp = pressure * pow(1.0 - (lapseRate * altitude) / (tempKelvin + lapseRate * altitude), exponent);
  return slp;
}

// Function to calculate dew point (°C)
float calculateDewPoint(float temperature, float humidity) {
  // Validate inputs
  if (isnan(temperature) || isnan(humidity) || humidity < 0 || humidity > 100 || temperature < -40 || temperature > 80) {
    Serial.printf("Invalid input: temperature=%.2f °C, humidity=%.2f %%\n", temperature, humidity);
    return NAN;
  }
  // Magnus-Tetens approximation
  float alpha = log(humidity / 100.0) + (17.625 * temperature) / (temperature + 243.04);
  float dewPoint = (243.04 * alpha) / (17.625 - alpha);
  return dewPoint;
}

// Function to calculate heat index (°C)
float calculateHeatIndex(float temperature, float humidity) {
  // Validate inputs
  if (isnan(temperature) || isnan(humidity) || humidity < 0 || humidity > 100 || temperature < -40 || temperature > 80) {
    Serial.printf("Invalid input: temperature=%.2f °C, humidity=%.2f %%\n", temperature, humidity);
    return NAN;
  }
  // Convert temperature to Fahrenheit for NOAA formula
  float tempF = (temperature * 9.0 / 5.0) + 32.0;
  float hiF;
  
  // NOAA Rothfusz regression formula
  if (tempF < 80.0 || humidity < 40.0) {
    hiF = tempF; // No heat index adjustment below 80°F or 40% humidity
  } else {
    hiF = -42.379 + 2.04901523 * tempF + 10.14333127 * humidity
          - 0.22475541 * tempF * humidity - 0.00683783 * tempF * tempF
          - 0.05481717 * humidity * humidity + 0.00122874 * tempF * tempF * humidity
          + 0.00085282 * tempF * humidity * humidity - 0.00000199 * tempF * tempF * humidity * humidity;
    
    // Adjustments for edge cases
    if (humidity < 13.0 && tempF >= 80.0 && tempF <= 112.0) {
      hiF -= ((13.0 - humidity) / 4.0) * sqrt((17.0 - abs(tempF - 95.0)) / 17.0);
    }
    if (humidity > 85.0 && tempF >= 80.0 && tempF <= 87.0) {
      hiF += ((humidity - 85.0) / 10.0) * ((87.0 - tempF) / 5.0);
    }
  }
  
  // Convert back to Celsius
  float hiC = (hiF - 32.0) * 5.0 / 9.0;
  return hiC;
}

// Function to read BME280 sensor data and calculate additional metrics
void readBME280() {
  float temperature = bme.readTemperature(); // Celsius
  float humidity = bme.readHumidity(); // Percentage
  float pressure = bme.readPressure() / 100.0F; // hPa
  
  // Validate sensor readings
  if (isnan(temperature) || isnan(humidity) || isnan(pressure) || humidity < 0 || humidity > 100 || pressure <= 0 || temperature < -40 || temperature > 80) {
    Serial.printf("Invalid sensor readings: temperature=%.2f °C, humidity=%.2f %%, pressure=%.2f hPa\n", temperature, humidity, pressure);
    return; // Skip calculations and printing
  }
  
  // Calculate additional metrics
  float absHumidity = calculateAbsoluteHumidity(temperature, humidity);
  float seaLevelPressure = calculateSeaLevelPressure(pressure, temperature, ALTITUDE);
  float dewPoint = calculateDewPoint(temperature, humidity);
  float heatIndex = calculateHeatIndex(temperature, humidity);
  
  // Print all values
  Serial.printf("Temperature: %.2f °C\n", temperature);
  Serial.printf("Humidity: %.2f %%\n", humidity);
  Serial.printf("Pressure: %.2f hPa\n", pressure);
  Serial.printf("Absolute Humidity: %.2f g/m³\n", absHumidity);
  Serial.printf("Sea-Level Pressure: %.2f hPa\n", seaLevelPressure);
  Serial.printf("Dew Point: %.2f °C\n", dewPoint);
  Serial.printf("Heat Index: %.2f °C\n", heatIndex);
  Serial.flush();
}

// Function to send data to ThingSpeak
void sendToThingSpeak() {
  WiFiClient client;
  ThingSpeak.begin(client);
  float temperature = bme.readTemperature();
  float humidity = bme.readHumidity();
  float pressure = bme.readPressure() / 100.0F;
  
  // Validate sensor readings
  if (isnan(temperature) || isnan(humidity) || isnan(pressure) || humidity < 0 || humidity > 100 || pressure <= 0 || temperature < -40 || temperature > 80) {
    Serial.printf("Invalid sensor readings: temperature=%.2f °C, humidity=%.2f %%, pressure=%.2f hPa\n", temperature, humidity, pressure);
    return; // Skip sending data
  }
  
  float absHumidity = calculateAbsoluteHumidity(temperature, humidity);
  float seaLevelPressure = calculateSeaLevelPressure(pressure, temperature, ALTITUDE);
  float dewPoint = calculateDewPoint(temperature, humidity);
  float heatIndex = calculateHeatIndex(temperature, humidity);
  
  // Check if any calculated values are invalid
  if (isnan(absHumidity) || isnan(seaLevelPressure) || isnan(dewPoint) || isnan(heatIndex)) {
    Serial.println("Invalid calculated values, skipping ThingSpeak upload");
    return;
  }
  
  ThingSpeak.setField(1, temperature);
  ThingSpeak.setField(2, humidity);
  ThingSpeak.setField(3, pressure);
  ThingSpeak.setField(4, absHumidity);
  ThingSpeak.setField(5, seaLevelPressure);
  ThingSpeak.setField(6, dewPoint);
  ThingSpeak.setField(7, heatIndex);
  ThingSpeak.setStatus(FIRMWARE_VERSION); // Send firmware version as status
  int response = ThingSpeak.writeFields(THINGSPEAK_CHANNEL_NUMBER, THINGSPEAK_WRITE_API_KEY);
  if (response == 200) {
    Serial.println("ThingSpeak update successful");
    // Single LED flash for successful upload (active LOW)
    digitalWrite(ledPin, LOW);  // LED on
    delay(200);
    digitalWrite(ledPin, HIGH); // LED off
    delay(200);
  } else {
    Serial.println("ThingSpeak update failed: " + String(response));
  }
  Serial.flush();
}

void setup() {
  // Initialize serial for debugging
  Serial.begin(115200);
  delay(100); // Give Serial time to initialize
  Serial.println("Booting...");
  Serial.print("Firmware Version: ");
  Serial.println(FIRMWARE_VERSION);
  Serial.flush();

  // Initialize LED pin
  Serial.println("Initializing LED...");
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, HIGH); // LED off (active LOW)
  Serial.flush();

  // Initialize BME280
  Serial.println("Initializing BME280...");
  if (!bme.begin(0x76)) { // Default I2C address for BME280 is 0x76 or 0x77
    Serial.println("Could not find a valid BME280 sensor, check wiring or I2C address!");
    while (1) {
      // Blink LED 3 times fast, then pause
      for (int i = 0; i < 3; i++) {
        digitalWrite(ledPin, LOW);  // LED on
        delay(200);
        digitalWrite(ledPin, HIGH); // LED off
        delay(200);
      }
      delay(2000); // Pause for 2 seconds
    }
  }
  Serial.println("BME280 initialized");
  Serial.flush();

  // Connect to WiFi
  Serial.println("Connecting to WiFi...");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  unsigned long wifiTimeout = millis() + 15000; // 15-second timeout
  while (WiFi.status() != WL_CONNECTED && millis() < wifiTimeout) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("WiFi connection failed! Retrying...");
    delay(1000);
    ESP.restart();
  }
  Serial.println("WiFi connected");
  Serial.flush();
  delay(2000); // Wait for WiFi to stabilize

  // Verify WiFi connection stability
  Serial.println("Verifying WiFi stability...");
  for (int i = 0; i < 3; i++) {
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WiFi disconnected! Retrying...");
      delay(1000);
      ESP.restart();
    }
    delay(500);
  }
  Serial.println("WiFi connection stable");
  Serial.flush();

  // Initialize NTP client
  Serial.println("Initializing NTP client...");
  timeClient.begin();
  if (WiFi.status() == WL_CONNECTED) {
    timeClient.update();
    if (timeClient.isTimeSet()) {
      Serial.print("Current time: ");
      Serial.println(timeClient.getFormattedTime());
    } else {
      Serial.println("NTP time sync failed, using internal RTC for timing");
    }
  } else {
    Serial.println("WiFi not connected, skipping NTP sync");
  }
  Serial.flush();

  // Initialize OTA
  Serial.println("Initializing OTA...");
  ArduinoOTA.setHostname("Sovde_Weather_Station"); // Set OTA hostname
  ArduinoOTA.setPassword(WIFI_PASSWORD); // Set OTA password to WiFi password
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else { // U_SPIFFS
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
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
    else Serial.println("Unknown Error");
    ESP.restart();
  });

  ArduinoOTA.begin();
  Serial.println("OTA Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  Serial.flush();
}

void loop() {
  ArduinoOTA.handle(); // Handle OTA updates

  // Update NTP time
  timeClient.update();

  if (timeClient.isTimeSet()) {
    // NTP time is available, check for 5-minute intervals
    static int lastMinute = -1;
    int currentMinute = timeClient.getMinutes();
    if (currentMinute % 5 == 0 && currentMinute != lastMinute) {
      Serial.print("Measurement at (NTP): ");
      Serial.println(timeClient.getFormattedTime());
      readBME280();
      sendToThingSpeak(); // Send data to ThingSpeak
      lastMinute = currentMinute;
    }
  } else {
    // NTP time not available, use internal RTC (millis) for 5-minute intervals
    static unsigned long lastReadTime = 0;
    if (millis() - lastReadTime >= 300000) { // 5 minutes = 300,000 ms
      Serial.println("Measurement at (RTC): 5-minute interval");
      readBME280();
      sendToThingSpeak(); // Send data to ThingSpeak
      lastReadTime = millis();
    }
  }
}
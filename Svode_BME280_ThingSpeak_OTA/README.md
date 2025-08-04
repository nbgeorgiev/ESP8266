# Svode Weather Station

This firmware runs on an ESP8266-based NodeMCU 0.9 (ESP-12 Module) to collect environmental data using a BME280 sensor and upload it to ThingSpeak. It supports Over-The-Air (OTA) updates for easy firmware upgrades. The firmware measures temperature, humidity, pressure, absolute humidity, sea-level pressure, dew point, and heat index, sending data to ThingSpeak every 5 minutes using NTP for accurate timing. The device is located at Svode, Bulgaria.

[Thingspeak channel](https://thingspeak.mathworks.com/channels/1873261)

## Features

- Sensor Readings: Collects **temperature** (°C), **relative humidity** (%), and **absolute pressure** (hPa) from a BME280 sensor via I2C.
- Calculated Metrics:
  - **Absolute Humidity** (g/m³) using the Tetens formula.
  - **Sea-Level Pressure** (hPa) adjusted for altitude (210m).
  - **Dew Point** (°C) using the Magnus-Tetens approximation.
  - **Heat Index** (°C) using the NOAA Rothfusz regression formula.
- ThingSpeak Integration: Uploads 7 fields to a ThingSpeak channel every 5 minutes.
- OTA Updates: Supports wireless firmware updates using ArduinoOTA.
- NTP Synchronization: Uses NTP for precise 5-minute intervals, with RTC fallback.
- LED Feedback: Built-in LED (GPIO2) flashes for successful ThingSpeak uploads or indicates BME280 errors.
- Input Validation: Checks for valid sensor readings to prevent erroneous data.
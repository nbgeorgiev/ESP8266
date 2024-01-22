# Bosch BME280 weather instrument

## Description
A bare ESP8266(ESP12F) module is connected to Bosch BME280 sensor to collect weather data. The MCU board is powered by 18650 LiPo rechargeable battery which is charged over some time. Using a voltage divider the battery voltage is monitored and an alarm is set up for critical level.

## Collected data
The Bosch BME280 sensor is collecting the following parameters - Temperature, Relative humidity and Absolute pressure. Using this data additionally the Absolute humidity, Relative barometric pressure(at sea level), and the Dew point are calculated. 

The data collected is pushed to a Thingspeak channel. Additionally the battery level is also reported to the channel field.

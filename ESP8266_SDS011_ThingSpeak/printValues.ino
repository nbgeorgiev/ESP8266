// Printing the data into the Serial Monitor for debugging

void printValues () {
  if (DEBUG) {

    // Data from SDS011
    Serial.println("Data from SDS011 sensor:");
    Serial.print("PM2.5 = ");
    Serial.print(pm25);
    Serial.print(" μg/m3");
    Serial.print(", PM10 = ");
    Serial.print(pm10);
    Serial.println(" μg/m3");

    //AQI index calculated
    Serial.println("AQI Index Calculated from measurments of PM2.5 and PM10: ");
    Serial.print("AQI PM2.5: ");
    Serial.print(aqi25);Serial.print(" ");
    Serial.print("AQI PM10: ");
    Serial.print(" "); 
    Serial.print(aqi10);
    Serial.print("  ");
    Serial.println("=============================================");
  }
}

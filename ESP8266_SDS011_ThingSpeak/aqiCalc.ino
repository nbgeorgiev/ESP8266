//AQI index calculation function

void aqiCalc () {
/* Initialization of upper and lower bounds of categories.
Note: Equation for AQI is a piecewise linear function, represented in
I = ((iHigh - iLow) / (cHigh - cLow)) * (c - cLow) + iLow
Where:
iHigh/iLow: Upper and lower bounds of the indices (e.g. 100-200)
cHigh/cLow: Upper and lower bounds of the concentrations (e.g. 12.1-35.4)
c: Current concentration of pollutant (pm25avg or pm10avg) */

   float iHigh = 0;
   float iLow = 0;
   float cHigh = 0;
   float cLow = 0;

   //Cascading if conditionals for the piecewise linear function (PM2.5)
   if (pm25 >= 350.5 && pm25 <= 500.4) {
      cLow = 350.5;
      cHigh = 500.4;
      iLow = 401;
      iHigh = 500;
   } else if (pm25 >= 250.5 && pm25 <= 350.4) {
      cLow = 250.5;
      cHigh = 350.4;
      iLow = 301;
      iHigh = 400;
   } else if (pm25 >= 150.5 && pm25 <= 250.5) {
      cLow = 150.5;
      cHigh = 250.4;
      iLow = 201;
      iHigh = 300;
   } else if (pm25 >= 55.5 && pm25 <= 150.5) {
      cLow = 55.5;
      cHigh = 150.4;
      iLow = 151;
      iHigh = 200;
   } else if (pm25 >= 35.5 && pm25 <= 55.5) {
      cLow = 35.5;
      cHigh = 55.4;
      iLow = 101;
      iHigh = 150;
   } else if (pm25 >= 12.1 && pm25 <= 35.5) {
      cLow = 12.1;
      cHigh = 35.4;
      iLow = 51;
      iHigh = 100;
   } else if (pm25 <= 12.1) {
      cLow = 0;
      cHigh = 12;
      iLow = 0;
      iHigh = 50;
   } else {
      Serial.println("The value for PM25 is wrong!\n");
   }

   // Calculate PM2.5 value for final output
   aqi25 = round(((iHigh - iLow) / (cHigh - cLow)) * (pm25 - cLow) + iLow);

   // Reset calculated values for the next caclulation (PM10)
   iLow = 0;
   iHigh = 0;
   cLow = 0;
   cHigh = 0;   

   //Define AQI category from PM2.5 measured values
   if (aqi25 >= 401 && aqi25 <= 500) {
      aqiCategory25 = "Hazardous";
   } else if (aqi25 >= 301 && aqi25 <= 400) {
      aqiCategory25 = "Hazardous";
   } else if (aqi25 >= 201 && aqi25 <= 300) {
      aqiCategory25 = "Very unhealthy";
   } else if (aqi25 >= 151 && aqi25 <= 200) {
      aqiCategory25 = "Unhealthy";
   } else if (aqi25 >= 101 && aqi25 <= 150) {
      aqiCategory25 = "Unhealthy for Sensitive Groups";
   } else if (aqi25 >= 51 && aqi25 <= 100)  {
      aqiCategory25 = "Moderate";
   } else if (aqi25 <= 50) {
      aqiCategory25 = "Good";
   } 

   //Cascading if conditionals for the piecewise linear function (PM10)
   if (pm10 >= 505 && pm10 <= 604) {
      cLow = 505;
      cHigh = 604;
      iLow = 401;
      iHigh = 500;
   } else if (pm10 >= 425 && pm10 <= 504) {
      cLow = 425;
      cHigh = 504;
      iLow = 301;
      iHigh = 400;
   } else if (pm10 >= 355 && pm10 <= 424) {
      cLow = 355;
      cHigh = 424;
      iLow = 201;
      iHigh = 300;
   } else if (pm10 >= 255 && pm10 <= 354) {
      cLow = 255;
      cHigh = 354;
      iLow = 151;
      iHigh = 200;
   } else if (pm10 >= 155 && pm10 <= 254) {
      cLow = 155;
      cHigh = 254;
      iLow = 101;
      iHigh = 150;
   } else if (pm10 >= 55 && pm10 <= 154) {
      cLow = 55;
      cHigh = 154;
      iLow = 51;
      iHigh = 100;
   } else if (pm10 <= 54) {
      cLow = 0;
      cHigh = 54;
      iLow = 0;
      iHigh = 50;
   } else {
      Serial.println("The value for PM10 is wrong!\n");
   }

   // Calculate PM10 variable for final output
   aqi10 = round(((iHigh - iLow) / (cHigh - cLow)) * (pm10 - cLow) + iLow);

   //Define AQI category from PM10 measured values
   if (aqi10 >= 401 && aqi10 <= 500) {
      aqiCategory10 = "Hazardous";
   } else if (aqi10 >= 301 && aqi10 <= 400) {
      aqiCategory10 = "Hazardous";
   } else if (aqi10 >= 201 && aqi10 <= 300) {
      aqiCategory10 = "Very unhealthy";
   } else if (aqi10 >= 151 && aqi10 <= 200) {
      aqiCategory10 = "Unhealthy";
   } else if (aqi10 >= 101 && aqi10 <= 150) {
      aqiCategory10 = "Unhealthy for Sensitive Groups";
   } else if (aqi10 >= 51 && aqi10 <= 100)  {
      aqiCategory10 = "Moderate";
   } else if (aqi10 <= 50) {
      aqiCategory10 = "Good";
   }  

   // Reset calculation variables for the next caclulation PM10
   iLow = 0;
   iHigh = 0;
   cLow = 0;
   cHigh = 0;
}

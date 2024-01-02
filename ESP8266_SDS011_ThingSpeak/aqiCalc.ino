// Float values comparison
bool cmpf(float A, float B, float epsilon = 0.05f)
{
    return (fabs(A - B) < epsilon);
}

// AQI value calculation formula
int aqiValue(float iHigh, float iLow, float cHigh, float cLow, float pm)
{
   /*Note: Equation for AQI is a piecewise linear function, represented in
   I = ((iHigh - iLow) / (cHigh - cLow)) * (pm - cLow) + iLow
   Where:
   iHigh/iLow: Upper and lower bounds of the indices (e.g. 100-200)
   cHigh/cLow: Upper and lower bounds of the concentrations (e.g. 12.1-35.4)
   pm: Current concentration of pollutant (pm25avg or pm10avg) */
   return round(((iHigh - iLow) / (cHigh - cLow)) * (pm - cLow) + iLow);
}

// Logic for AQI coefficients used for calculation
int aqiCalc(float pm, const float pm_limits[]){

   float iHigh = 0;
   float iLow = 0;
   float cHigh = 0;
   float cLow = 0;
   int aqi = 0;
   // Initialization of upper and lower bounds of categories.
   const float aqi_iLowHigh[8] = {0, 50, 100, 150, 200, 300, 400, 500};

   if((pm < pm_limits[1]) || cmpf(pm, pm_limits[1])){
      iHigh = aqi_iLowHigh[1];
      iLow = aqi_iLowHigh[0];
      cHigh = pm_limits[1];
      cLow = pm_limits[0];
   } else if(pm > pm_limits[7]){
      iHigh = aqi_iLowHigh[7];
      iLow = aqi_iLowHigh[6];
      cHigh = pm_limits[7];
      cLow = pm_limits[6];
   } else if (pm < 0) {
      //Error in SDS011 sensor readings
      aqi = -1;
      return aqi;
   } else {
      for(int i = 1; i < 7; i++)
      {
         if((pm < pm_limits[i+1]) || (cmpf(pm, pm_limits[i+1])))
         {
            iHigh = aqi_iLowHigh[i+1];
            iLow = aqi_iLowHigh[i];
            cHigh = pm_limits[i+1];
            cLow = pm_limits[i];
            break;
         }
      }
   }
   aqi = aqiValue(iHigh, iLow, cHigh, cLow, pm);
   return aqi;
}

//AQI index calculation function
void aqi(float pm25, float pm10, int* aqi25, int* aqi10) {
   // Breakpoints for the AQI
   const float pm25_conc_values[8] = {0, 12.0, 35.4, 55.4, 150.4, 250.4, 350.4, 500.4};
   const float pm10_conc_values[8] = {0, 54, 154, 254, 354, 424, 504, 604};

   *aqi25 = aqiCalc(pm25, pm25_conc_values);
   *aqi10 = aqiCalc(pm10, pm10_conc_values);
}
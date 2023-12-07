// Pushing data to Thingspeak.com
#include <ThingSpeak.h>

void thingSpeak () {
  // Initialize ThingSpeak
  ThingSpeak.begin(client);
  // Set the fields with the values
  ThingSpeak.setField(1, pm25);
  ThingSpeak.setField(2, pm10);
  ThingSpeak.setField(3, aqi25);
  ThingSpeak.setField(4, aqi10);

  // Write to ThingSpeak. There are up to 8 fields in a channel, allowing you to store up to 8 different
  // pieces of information in a channel.  Here, we write to field 1.
  int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

  if(x == 200){
    Serial.println("Channel updated successfully into Thingspeak.");
    Serial.println("============================================="); 
  } else {
    Serial.println("Problem updating channel. HTTP error code " + String(x));
  }
}

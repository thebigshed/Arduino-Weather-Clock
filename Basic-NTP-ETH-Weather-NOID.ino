// Set the Arduino clock to rtc every 5 mins
// Get the weather from OpenWeatherMap at the same time
// Update the RTC with NTP time at 02:02:00 every day
// Whilst this is overkill it did give me a chance to play with the different functions
// Things to consider:
//    Add an alarm
//    Turn off the display at night
//    Replace the hardcoded APPID and Location with something dynamic
// As it was only for my use these are not big issues

#include <NTPClient.h>
#include <LiquidCrystal_I2C.h>
#include <DS3232RTC.h>
#include <Ethernet.h>
#include <EthernetUdp.h>
#include <ArduinoJson.h>
#include <SPI.h>

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

EthernetUDP ntpUDP;
NTPClient timeClient(ntpUDP);

LiquidCrystal_I2C lcd(0x27, 4, 20);

void setup() {
  Serial.begin(115200);


  Ethernet.begin(mac); // Start the network

  lcd.init(); // start the LCD

  lcd.backlight(); //Turn on the backlight;

  setSyncProvider(RTC.get);   // the function to get the time from the RTC

  if (timeStatus() != timeSet)
    Serial.println("Unable to sync with the RTC");
  else
    Serial.println("RTC has set the system time");

}

void loop() {

  lcd_time();

  // only get the temp every 5 mins to ensure we do not break the openweathermap rules for a free contract.

  int latch = 0;
  // Every 5 mins go and get the temp but ensure it only does it once
  if ((minute() % 5 == 0) && (second() % 60 == 0) && (latch == 0)) {
    get_temperature();
    Serial.println("Get Temp");
    latch = 1;
    setSyncProvider(RTC.get);   // the function to get the time from the RTC

    if (timeStatus() != timeSet)
      Serial.println("Unable to sync with the RTC");
    else
      Serial.println("RTC has set the system time");
  }

  if ((minute() % 5 != 0) && (second() % 60 == 0) && (latch == 1)) {
    latch = 0 ;
    //Serial.println("reset latch");
  }
  int counter = 0;
  // Set the RTC to the NTP time @ 02:02:00 so that it is not being watched
  if (( hour() == 2 ) && (minute() == 2 )) {
    counter = set_rtc_to_ntp(counter);
    Serial.println("Set RTC to NTP");
    //Serial.println(counter);
  }

  if ((hour() == 2) && (minute() == 3 ) && (counter == 1)) {
    counter = 0;
    //Serial.println(counter);
  }

}


// I am only interested in the current temperature on this request
// you have to use your location after the q
// and your ID from openweathermap for appid

void get_temperature () {
  // Connect to HTTP server
  while (Ethernet.linkStatus() == LinkOFF) {
    Ethernet.begin(mac); // Start the network
    delay ( 500 );
    Serial.print ( "." );
  }
  EthernetClient client;
  client.setTimeout(500);
  if (!client.connect("api.openweathermap.org", 80)) {
    Serial.println(F("Connection failed"));
    return;
  }
  // Send HTTP request
  client.println(F("GET /data/2.5/weather?q=XXXXXXXX,XXX&units=metric&appid=XXXXXXXXXXXXXXXXXXXXXXXX"));
  client.println(F("Host: api.openweathermap.org"));
  client.println(F("Connection: close"));
  if (client.println() == 0) {
    Serial.println(F("Failed to send request"));
    client.stop();
    return;
  }


  // Check HTTP status & read the reply in to status
  char status[512] = {0};
  client.readBytesUntil('\r', status, sizeof(status));

  // check i got the whole string
  if (strcmp(status + ((strlen(status)) - 4) , "200}") != 0) {
    Serial.println(F("Unexpected response: "));
    //Serial.println(status);
    client.stop();
    return;
  }

  // The filter: it contains "true" for each value we want to keep
  StaticJsonDocument<500> doc;
  // Deserialize the JSON document
  DeserializationError error = deserializeJson(doc, status);

  // Test if parsing succeeds.
  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.f_str());
    return;
  }

  // Fetch values.
  //
  String sensor = doc["main"]["temp"];
  String description = doc["weather"][0]["description"];

  //Serial.println(description);
  //used for testing the decoding:
  //double latitude = doc["coord"]["lat"];
  //double longitude = doc["coord"]["lon"];
  //Print values.
  //Serial.println(sensor);
  //Serial.println(latitude, 6);
  //Serial.println(longitude, 6);
  char buf2[20]; // one line on the LCD
  sprintf(buf2, "%S \xDF C.       ", sensor);
  lcd.setCursor(0, 2);
  lcd.print(buf2);
  char buf3[20]; // one line on the LCD
  sprintf(buf3, "%S           ", description);
  lcd.setCursor(0, 3);
  lcd.print(buf3);
}

int set_rtc_to_ntp(int local_counter) {
  while (Ethernet.linkStatus() == LinkOFF) {
    delay ( 500 );
    Serial.print ( "." );
  }
  Serial.println("Ethernet is up");
  if (local_counter == 0 ) {
    timeClient.begin();
    timeClient.update();
    time_t utcCalc = timeClient.getEpochTime();
    DS3232RTC rtc_clock;
    rtc_clock.set(utcCalc);
    Serial.println("RTC Set to NTP");
    return (1);
  }
  else {
    return (2);
  }

}


void lcd_time() {
  char buf1[20]; // one line on the LCD
  sprintf(buf1, "%02d:%02d:%02d  %02d/%02d/%02d",  hour(), minute(), second(), day(), month(), year());
  lcd.setCursor(0, 0);
  lcd.print(buf1);

}

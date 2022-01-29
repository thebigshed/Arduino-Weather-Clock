# Arduino-Weather-Clock
A 20x4 Display which shows the date, time, temp and weather description

The DS3232 is sync with NTP at 02:02:00 every day.

The weather is obtained from OpenWeatherMap every 5 mins and also the RTC updates the system time.

You will need an sccount on OpenWeatherMap to get the APPID and decide the location you wish to use. These are hardcoded and replaced with XXXXXXX in this version.

It means you will have to change them to get the code to work.

The Libraries are specific to my chosen DS3232.

Have fun...

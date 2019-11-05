
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

RTC_DS1307 rtc;

Adafruit_BME280 bme;

const int chipSelect = 10; // defaul chip select from datalogger shield

void setup () {
  
  Serial.begin(9600);
  
  if (!rtc.begin()) {
    Serial.println("RTC is not connected");
    while (1);
  }

  if (!rtc.isrunning()) {
    Serial.println("RTC is not running");
  }
  
  if (!SD.begin(chipSelect)) {
    Serial.println("SD card failed, or not present");
    return;
  }

  if (!bme.begin()) {
      Serial.println("Could not find a valid BME280");
      while (1);
  }
  
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  File dataFile = SD.open("logger.txt", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    // dataFile.println("Time;Temp;Hum;Prs"); // write the first row
    dataFile.close();
  }
  
  Serial.println("Time;Temp;Hum;Prs"); // write the first row
}

void loop () {
  DateTime now = rtc.now();
  
  char dateBuffer[12];
  char sensorBuffer[12];

  sprintf(dateBuffer, "%04u/%02u/%02u %02u:%02u:%02u",
    now.year(), now.month(), now.day(),
    now.hour(), now.minute(), now.second());
  
  Serial.print(dateBuffer);

  Serial.print(";");
  Serial.print(bme.readTemperature());
  Serial.print(";");
  Serial.print(bme.readHumidity());
  Serial.print(";");
  Serial.println(bme.readPressure() / 100.0F);

  File dataFile = SD.open("logger.txt", FILE_WRITE);

  if (dataFile) {
    
    dataFile.print(dateBuffer);
    dataFile.print(";");
    dataFile.print(bme.readTemperature());
    dataFile.print(";");
    dataFile.print(bme.readHumidity());
    dataFile.print(";");
    dataFile.println(bme.readPressure() / 100.0F);
    dataFile.close();
  }
  
  delay(1000);
}

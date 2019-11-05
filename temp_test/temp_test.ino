#include <Time.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <DS3231.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

Adafruit_BME280 bme;
DS3231  rtc(SDA, SCL);
const int chipSelect = 7;

void setup() {

  Serial.begin(9600);
  if (!bme.begin(0x76)) {
    Serial.println("BME280 sensor not found");
    while (1);
  }
  //Serial.println("Time;Temp;Hum;Prs");
  rtc.begin();

  // uncomment to set the date and time
  //rtc.setDOW(FRIDAY);     // Set Day-of-Week
  //rtc.setTime(17, 5, 30);     // Set the time HH,MM,SS
  //rtc.setDate(15, 3, 2019);   // Set the date DD,MM,YYYY

    if (!SD.begin(chipSelect)) {
    Serial.println("card failed, or not present");
    // don't do anything more:
    return;
  }

  // open the file.
  File dataFile = SD.open("logger.txt", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    //dataFile.println("Time;Temp;Hum;Prs"); // write the first row
    dataFile.close();
  }
  
  Serial.println("Time;Temp;Hum;Prs"); // write the first row

}

void loop() {
  Serial.print(rtc.getDateStr());
  Serial.print(" ");
  Serial.print(rtc.getTimeStr());
  Serial.print(";");
  Serial.print(bme.readTemperature());
  Serial.print(";");
  Serial.print(bme.readHumidity());
  Serial.print(";");
  Serial.println(bme.readPressure() / 100.0F);

  File dataFile = SD.open("logger.txt", FILE_WRITE);

  if (dataFile) {
    
    dataFile.print(rtc.getDateStr());
    dataFile.print(" ");
    dataFile.print(rtc.getTimeStr());
    dataFile.print(";");
    dataFile.print(bme.readTemperature());
    dataFile.print(";");
    dataFile.print(bme.readHumidity());
    dataFile.print(";");
    dataFile.println(bme.readPressure() / 100.0F);
    dataFile.close();
  }

  
  delay(5000);
}

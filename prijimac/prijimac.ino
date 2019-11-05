#include <VirtualWire.h> // use Virtual library for decode signal from Rx module
#include "RTClib.h"
#include <SPI.h>
#include <SD.h>

// Clock
RTC_DS3231 rtc;

// Sensors
double hum;
double temp;
double pres;

// SD pin
const int chipSelect = 10;

char MsgReceived[42];

int led = 2; // pin for LED

void setup()
{
  Serial.begin(9600);

  pinMode(led, OUTPUT);

  // VirtualWire
  // Bits per sec
  vw_setup(2000);
  // set pin for connect receiver module
  vw_set_rx_pin(A0);
  // Start the receiver PLL running
  vw_rx_start();

  rtc.begin();
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
  }

  File dataFile = SD.open("logger.txt", FILE_WRITE);
  // if the file is available, write to it:
  if (dataFile) {
    // dataFile.println("Time;Temp;Hum;Prs"); // write the first row
    dataFile.close();
  }

  Serial.println("Time;Temp;Hum;Prs"); // write the first row

}

void loop()
{
  uint8_t buf[VW_MAX_MESSAGE_LEN];
  uint8_t buflen = VW_MAX_MESSAGE_LEN;

  char bufdate[12];

  // Taking the data from the control base
  if (vw_get_message(buf, &buflen))
  {
    digitalWrite(led, HIGH);

    delay(100);

    int i;
    // Message with a good checksum received, dump it.
    for (i = 0; i < buflen; i++)
    {
      // Fill Msg Char array with corresponding
      // chars from buffer.
      MsgReceived[i] = char(buf[i]);
    }

    DateTime now = rtc.now();

    //sprintf(bufdate, "%04u/%02u/%02u %02u:%02u:%02u",
    //        now.year(), now.month(), now.day(),
    //        now.hour(), now.minute(), now.second());

    //Serial.print(bufdate);

    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.print(";");
    Serial.println(MsgReceived);

    File dataFile = SD.open("logger.txt", FILE_WRITE);

    if (dataFile) {

      dataFile.print(now.year(), DEC);
      dataFile.print('/');
      dataFile.print(now.month(), DEC);
      dataFile.print('/');
      dataFile.print(now.day(), DEC);
      dataFile.print(" ");
      dataFile.print(now.hour(), DEC);
      dataFile.print(':');
      dataFile.print(now.minute(), DEC);
      dataFile.print(':');
      dataFile.print(now.second(), DEC);
      dataFile.print(";");
      dataFile.println(MsgReceived);
      dataFile.close();
    }

    digitalWrite(led, LOW);
    memset(MsgReceived, 0, sizeof(MsgReceived)); // This line resets the StringReceived
    //memset(bufdate, 0, sizeof(bufdate));
  }

}

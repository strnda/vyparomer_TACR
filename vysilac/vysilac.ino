// 433 MHz vysílač

// připojení knihovny
#include <VirtualWire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include "LowPower.h"

Adafruit_BME280 bme; // I2C

int ledPin = 13;
char Msg[42];// The string to be send trought rf transmitter 
 
void setup() 
{
  // Serial.begin(9600);
  
  bme.begin(); // Sensor ini

  pinMode(ledPin,OUTPUT);
  
  vw_setup(2000); // Bits per sec
  vw_set_tx_pin(12);// Set the Tx pin. Default is 12
}
 
void loop() 
{
 
  // Read and store Sensor Data
  float hum = bme.readHumidity();
  float temp = bme.readTemperature();
  float pres = bme.readPressure() / 100.0F;

  int hum1 = (int)hum;
  int hum2 = (hum - hum1) * 100; // For two decimal points
  
  int temp1 = (int)temp;
  int temp2 = (temp - temp1) * 100; // For two decimal points

  int pres1 = (int)pres;
  int pres2 = (pres - pres1) * 100; // For two decimal points
    
  sprintf(Msg, "%i.%i;%i.%i;%i.%i", 
                hum1, hum2,
                temp1, temp2,
                pres1, pres2);

  // Serial.println(Msg);
 
  // Turn on a light to show transmitting
  digitalWrite(ledPin, HIGH); 
 
  //LowPower.powerDown(SLEEP_250MS, ADC_OFF, BOD_OFF);  
  delay(100); 
  
  vw_send((uint8_t *)Msg, strlen(Msg));
  vw_wait_tx(); // Wait until the whole message is gone
 
  // Turn off a light after transmission
  
  digitalWrite(ledPin, LOW); 
  
  // put 5 mins sleep mode
  // As lowpower library support maximam 8s ,we use for loop to take longer (5mins) sleep
  // 5x60=300
  // 300/4=75
  for(int i = 0; i < 5; i++)
  {
    LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);    // Instead of delay(4000); 
    //delay(4000);
  }
}

/*-----------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------Vyparomer FIRMWARE----------------------------------------------------*/
/*---------------------------------------------------by------------------------------------------------------------*/
/*------------------------------------------------Jan-Cadek--------------------------------------------------------*/
/*-----------------------------------------------------------------------------------------------------------------*/
/* SD card reader
  PINOUT:
  GND - x
  +3.3 - 3.3V
  +5 - x
  CS - PIN 53
  MOSI - PIN 51
  SCK - PIN 52
  MISO - PIN 50
  GND - GND
*/
#include <SdFat.h>
#define chipSelect 53 // CS PIN 53 pro SD card
SdFat SD;
/*-----------------------------------------------------------------------------------------------------------------*/





/*-----------------------------------------------------------------------------------------------------------------*/
/* BME280 forced (low power) - vzduch: teplota, vlhkost, tlak
  PINOUT:
  VIN - 5V
  GND - GND
  SCL - SCL (PIN 21)
  SDA - SDA (PIN 20)
*/
//#include <forcedClimate.h>
//ForcedClimate BME280 = ForcedClimate();

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
// nastavení adresy senzoru BME280
#define BME280_ADRESA (0x76)
Adafruit_BME280 BME280;
/*-----------------------------------------------------------------------------------------------------------------*/





/*-----------------------------------------------------------------------------------------------------------------*/
/* BDSensor + TTL/RS485 prevodnik
  PINOUT:
  BDSM white - 5V
  BDSM brown -  GND
  BDSM pink - B (converter)
  BDSM yellow - A (converter)
  RS485/TTL VCC - 5V
  RS485/TTL GND - GND
  RS485/TTL RO - PIN 19 (RX1)
  RS485/TTL RE - nutno spojit s DE!!!
  RS485/TTL DE - PIN 17
  RS485/TTL DI - PIN 18 (TX1)
*/
#include <SensorModbusMaster.h>
byte modbusAddress = 0x01;
const int DEREPin = 17;   // The pin controlling Recieve Enable and Driver Enable
modbusMaster modbus;
/*-----------------------------------------------------------------------------------------------------------------*/





/*-----------------------------------------------------------------------------------------------------------------*/
/* Indikacni LED RGB
  PINOUT:
  +5V - 5V
  Din - PIN 5
  GND - GND
*/
#include <Adafruit_NeoPixel.h>
#define LEDpin 5 //datový PIN Din
#define NUMPIXELS 1 //pocet LED
Adafruit_NeoPixel pixels(NUMPIXELS, LEDpin, NEO_GRB + NEO_KHZ800);
/*-----------------------------------------------------------------------------------------------------------------*/





/*-----------------------------------------------------------------------------------------------------------------*/
/* DS18B20 - mereni teploty, pripojeni libovolneho poctu senzoru ke sbernici OneWire
  PINOUT:
  RED - +5V
  YELLOW - PIN 9
  BLACK - GND
  mezi RED a YELLOW nutno zapojit rezistor napr. 4k7
*/
#include <OneWire.h>
#include <DallasTemperature.h>
#define pinCidlaDS 9 // nastavení čísla vstupního pinu
OneWire oneWireDS(pinCidlaDS); // vytvoření instance oneWireDS z knihovny OneWire
DallasTemperature senzoryDS(&oneWireDS); // vytvoření instance senzoryDS z knihovny DallasTemperature
uint8_t pocetDS = 0; //vychozi pocet pripojenych DS18B20
/*-----------------------------------------------------------------------------------------------------------------*/





/*-----------------------------------------------------------------------------------------------------------------*/
/* RTC modul DS3231 - datum, cas, alarm (buzeni ze sleepmodu)
  PINOUT:
  32K - x
  SQW - x
  SCL - SCL (PIN 21)
  SDA - SDA (PIN 20)
  VCC - 5V - k aku
  GND - GND - k aku
*/
#include <DS3232RTC.h>
//#include <avr/sleep.h>
//#include <avr/power.h>
const int intervalMereni = 10; // Nastaveni intervalu mereni v minutach
//#define pinProbuzeni 2 //interrupt PIN
/*-----------------------------------------------------------------------------------------------------------------*/


String vysledkyMereni;

void setup()
{
  Serial.begin(9600); //Start Serial Comunication USB
  //Serial3.begin(9600); //Start seriove komunikace pro LoRa bezdratovy prenos (PINY 14 a 15)



  pinMode(LED_BUILTIN, OUTPUT); //povoleni ovladani integrovane LED na desce Arduino
  digitalWrite(LED_BUILTIN, LOW); //vypnuti integrovane LED pro nizsi spotrebu

  allAnalogPinsToLOW();
  allUnusedDigitalPinsToLOW();
  ShutDownADC();

  //setTime(15, 8, 00, 23, 11, 2021); //nastaveni casu
  //RTC.set(now()); //nastavi cas dle predchoziho radku, staci spustit jednou pri kompilaci, a pak zakomentovat, jinak se cas vzdy prepise

  RTC.squareWave(SQWAVE_NONE); //nastavi SQW pin na moznost interruptu
  RTC.alarmInterrupt(ALARM_1, true); //povoli Alarmu 1 provadet interrupt (vyslat signal k probuzeni)

  pixels.begin(); // inicializace RGB stavove LED
  pixels.clear();

  zablikej(100, 50, 0); //zablikej 2x zlute kdyz se zapne a chvili pockej
  delay(1000);

  Serial.print("Inicializace SD karty... "); //inicializace SD card - CS PIN 53
  if (!SD.begin(chipSelect)) {
    Serial.println("selhala!");
    zablikej(100, 0, 0); //blikej 2x cervene pri selhani
    delay(500);
    return;
  }
  else {
    zablikej(0, 100, 0); //zablikej 2x zelene kdyz OK a chvili pockej
    //delay(500);
    Serial.println("OK");
  }

  //zahájení komunikace s BME280
  Serial.print("Inicializace BME280... "); //inicializace SD card - CS PIN 53
  if (!BME280.begin(BME280_ADRESA)) {
    Serial.println("selhala!");
    zablikej(100, 20, 0); //blikej 2x oranzove pri selhani
    delay(500);
    return;
  }
  else {
    zablikej(0, 155, 120); //zablikej 2x tyrkysove kdyz OK a chvili pockej
    //delay(500);
    Serial.println("OK");
  }
  // BME280.begin(); //inicializace BME280

  senzoryDS.begin(); //inicializace DS18B20 senzoru teploty
  pocetDS = senzoryDS.getDeviceCount();  //detekuj pocet pripojenych senzoru DS18B20

  pinMode(DEREPin, OUTPUT); //nastaveni DEREPin - PIN 7 - jako vystup pro rizeni komunikace s BDSensor
  Serial1.begin(9600); //Start Serial Comunication RX1/TX1 pro BDSensor
  modbus.begin(modbusAddress, Serial1, DEREPin); //inicializace komunikace s BDSensor


}


void loop(void)
{

  nactiSenzory(); //nacti data ze senzoru a zapis je na SD kartu
  //Serial3.flush();
 // Serial3.println(vysledkyMereni);
  //delay(500);
  
  time_t cas = RTC.get(); //nastaveni dalsiho alarmu dle intervalMereni
  int casAlarmu = 0;
  if (minute(cas) < (60 - intervalMereni))   //osetreni casu alarmu pri prechodu na dalsi hodinu
  {
    casAlarmu = (minute(cas) + intervalMereni);
  }
  else
  {
    casAlarmu = (minute(cas) + intervalMereni) - 60;
  }

  delay(4000); //pauza 4 s a pak se vypne
  zablikej(100, 0, 100); //zablikej 3x ruzove kdyz jde spat
  RTC.setAlarm(ALM1_MATCH_MINUTES , 0, casAlarmu, 0, 0); // samotne nastaveni pristiho alarmu
  RTC.alarm(ALARM_1);
  
}



void nactiSenzory() {
  //  float temp = BME280.getTemperatureCelcius(true); //teplota vzduchu
  //  float hum = BME280.getRelativeHumidity(true); //vlhkost vzduchu
  //  float pres = BME280.getPressure(true); //relativni atmosfericky tlak

  float temp = BME280.readTemperature(); //teplota vzduchu
  float hum = BME280.readHumidity(); //vlhkost vzduchu
  float pres = (BME280.readPressure() / 100.0F); //relativni atmosfericky tlak

  float dpth = modbus.float32FromRegister(0x04, 0x08, bigEndian); //hloubka BDSensor
  float wtemp = modbus.float32FromRegister(0x04, 0x12, bigEndian); //teplota BDSensor
  String DStemps = nactiTeplotuVsechDS(); //teploty vsech pripojenych DS18B20

  zapisData(temp, hum, pres, dpth, wtemp, DStemps);

  //  Serial.print("Humidity: ");
  //  Serial.print(hum);
  //  Serial.print(" % ");
  //  Serial.print("Temperature: ");
  //  Serial.print(temp);
  //  Serial.print(" °C ");
  //  Serial.print("Pressure: ");
  //  Serial.print(pres);
  //  Serial.print(" hPa ");
  //  Serial.print("Depth: ");
  //  Serial.print(round(dpth)); //zaokrouhleno na celé milimetry
  //  Serial.print(" mm ");
  //  Serial.print("Water temperature BD: ");
  //  Serial.print(wtemp);
  //  Serial.print(" °C ");
  //  Serial.print("Water temperature DS18B20s: ");
  //  Serial.print(DStemps);
  //  Serial.print(" °C ");
  //  Serial.println();

}

void zapisData(float temp, float hum, float pres, float dpth, float wtemp, String DStemps) {
  time_t casMereni = RTC.get();

  //Serial.println(String(year(casMereni)) + "/" + String(month(casMereni)) + "/" + String(day(casMereni)) + " " + String(hour(casMereni)) + ":" + String(minute(casMereni)) + ":" + String(second(casMereni)));
  vysledkyMereni = String(year(casMereni)) + "/" + String(month(casMereni)) + "/" + String(day(casMereni)) + ";" + String(hour(casMereni)) + ":" + String(minute(casMereni)) + ":" + String(second(casMereni)) + ";" + String(temp) + ";" + String(hum) + ";" + String(pres) + ";" + String(dpth) + ";" + String(wtemp) + ";" + DStemps;

  Serial.println(vysledkyMereni); //vypis data do Serial Monitoru USB

  File logFile = SD.open("log.txt", FILE_WRITE);
  if (logFile) {
    logFile.println(vysledkyMereni);
    logFile.close();
    zablikej(0, 0, 100); //zablikej 3x modre kdyz OK a chvili pockej
    //delay(500);
  } else {
    Serial.println("Chyba zapisu dat na SD kartu!!!");
    inicializaceSD();
  }
 //String vypis = vysledkyMereni + "\r\n";
 //Serial3.print(vysledkyMereni);
 //Serial3.flush();
// delay(10000);
 //while(!Serial3.available());
//while(!Serial3.flush()=true);


  
  //while(!Serial3.available()){Serial3.readString(); delay(1000);};
//  String potvrzeni = "received";
//  String potvrzeniPrijate = Serial3.readString();
//  while(potvrzeni != potvrzeniPrijate) {
//     Serial3.println(vysledkyMereni);
//     Serial.println("neprijato");
//     delay(5000);
//     potvrzeniPrijate = Serial3.readString();
//  }
  
}

void zablikej(byte r, byte g, byte b) {        //zablikej 2x
  for (int i = 0; i < 2; i++) {
    pixels.setPixelColor(0, pixels.Color(r, g, b));
    pixels.show();
    delay(10);
    pixels.setPixelColor(0, pixels.Color(0, 0, 0));
    pixels.show();
    delay(150);
  }

}

void inicializaceSD() {
  Serial.print("Reinicializace SD karty... "); //inicializace SD card - CS PIN 53
  if (!SD.begin(chipSelect)) {
    Serial.println("selhala!");
    zablikej(100, 0, 0); //zablikej 3x cervene kdyz nelze otevrit logfile a zkus znovu nacist kartu
    //return;
  }
  else {
    zablikej(0, 100, 0); //zablikej 3x zelene kdyz OK
    Serial.println("OK");
  }
}

String nactiTeplotuVsechDS() { //automaticke nacitani teploty z DS18B20 - po pripojeni dalsi sondy nutno resetovat desku Arduino!
  senzoryDS.requestTemperatures();
  String teplotyVsechDS;
  for (uint8_t i = 0; i < pocetDS; i++)
  {
    if (i < pocetDS - 1) {
      teplotyVsechDS = teplotyVsechDS + String(senzoryDS.getTempCByIndex(i)) + ";";
    } else {
      teplotyVsechDS = teplotyVsechDS + String(senzoryDS.getTempCByIndex(i));
    }
  }
  return teplotyVsechDS;
}

void allAnalogPinsToLOW() {
  pinMode(A0, OUTPUT);
  pinMode(A1, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A5, OUTPUT);
  pinMode(A6, OUTPUT);
  pinMode(A7, OUTPUT);
  pinMode(A8, OUTPUT);
  pinMode(A9, OUTPUT);
  pinMode(A10, OUTPUT);
  pinMode(A11, OUTPUT);
  pinMode(A12, OUTPUT);
  pinMode(A13, OUTPUT);
  pinMode(A14, OUTPUT);
  pinMode(A15, OUTPUT);

  digitalWrite(A0, LOW);
  digitalWrite(A1, LOW);
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);
  digitalWrite(A4, LOW);
  digitalWrite(A5, LOW);
  digitalWrite(A6, LOW);
  digitalWrite(A7, LOW);
  digitalWrite(A8, LOW);
  digitalWrite(A9, LOW);
  digitalWrite(A10, LOW);
  digitalWrite(A11, LOW);
  digitalWrite(A12, LOW);
  digitalWrite(A13, LOW);
  digitalWrite(A14, LOW);
  digitalWrite(A15, LOW);

}

void allUnusedDigitalPinsToLOW() {
  pinMode(0, OUTPUT);
  pinMode(1, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(8, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(13, OUTPUT);
  //pinMode(14, OUTPUT);
  //pinMode(15, OUTPUT);
  pinMode(16, OUTPUT);
  pinMode(17, OUTPUT);

  digitalWrite(0, LOW);
  digitalWrite(1, LOW);
  digitalWrite(2, LOW);
  digitalWrite(4, LOW);
  digitalWrite(6, LOW);
  digitalWrite(8, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);
  digitalWrite(12, LOW);
  digitalWrite(13, LOW);
  //digitalWrite(14, LOW);
  //digitalWrite(15, LOW);
  digitalWrite(16, LOW);
  digitalWrite(17, LOW);

  for (int i = 23; i <= 49; i++) {
    pinMode(i, OUTPUT);
    digitalWrite(i, LOW);
  }
}

void  ShutDownADC(void)
{
  ACSR = (1 << ACD);                      // disable A/D comparator
  ADCSRA = (0 << ADEN);                   // disable A/D converter
  DIDR0 = 0x3f;                           // disable all A/D inputs (ADC0-ADC5)
  DIDR1 = 0x03;                           // disable AIN0 and AIN1
}

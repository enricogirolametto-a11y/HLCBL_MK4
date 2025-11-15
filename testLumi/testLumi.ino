#include "RTClib.h"

#ifdef __AVR__
#include <avr/power.h>  // Required for 16 MHz Adafruit Trinket
#endif


RTC_DS1307 rtc;









const int pinPRes_DX = A0;
const int pinPRes_SX = A1;

int sensorValue = 0;  // variable to store the value coming from the sensor
int numCalcMedia = 0;
int calcMediaTOT;
int secSwitch_ex;
 
 int calcMedia;


void setup() {

// ** RTC
#ifndef ESP8266
  while (!Serial)
    ;  // wait for serial port to connect. Needed for native USB
#endif
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  if (!rtc.isrunning()) {
    Serial.println("RTC is NOT running, let's set the time!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // declare the ledPin as an OUTPUT:
  Serial.begin(9600);
}

void loop() {
  
  DateTime now = rtc.now();
  int iSec = now.second();
  int iMin = now.minute();
  int HourOrig = now.hour();

  // read the value from the sensor:
  sensorValue =  calcLum(pinPRes_DX, pinPRes_SX, iSec);

  // stop the program for <sensorValue> milliseconds:
  delay(sensorValue);




}



int calcLum(int pin1, int pin2, int secSwitch) {
  // serve per fare la media delle luminosità in entrata e quindi regolare di conseguenza la luminosità dell'orologio
  int calcMedia;
  int calcMediaFinal;
  int adjustMin = 1;  // SERVE PER ASSICURARE IL VALORE MINIMO DI LUMINOSITà; dallo 0 al 1 sarà sempre il minimo
  int deltaLum = 2;   // il delta di intervento per cambiare la media
  int valLum1;
  int valLum2;
  int calcMedia_prop = 0;

  valLum1 = analogRead(pin1);
  valLum2 = analogRead(pin2);
  //valLum2 = valLum2 + adjustLum2;


  calcMediaTOT = calcMoreBig(valLum1, valLum2, adjustMin);
  

  if (secSwitch != secSwitch_ex) {
    calcMedia_prop = calcMediaTOT;

    if (calcMedia_prop < 1) {  //la luminosità minima assoluta
      calcMedia_prop = 1;
    }

    if ((calcMedia + deltaLum) < calcMedia_prop || (calcMedia - deltaLum) > calcMedia_prop) {
      calcMedia = calcMedia_prop;
      if (calcMedia <= 0) {
        calcMedia = 1;
      }
    }

    // gestisco con il dato real
  // do una serie di regole per cui la luminosità sarà affiadata ad uno swith case
  switch (calcMedia) {
    case 0 ... 4:
      calcMediaFinal = 3;
      break;
    case 5 ... 50:
      calcMediaFinal = 10;
      break;
    case 51 ... 90:
      calcMediaFinal = 20;
      break;
    case 91 ... 150:
      calcMediaFinal = 30;
      break;
    default:
      calcMediaFinal = 50;
  };

    secSwitch_ex = secSwitch;
    numCalcMedia = 0;
  }

  return calcMediaFinal;
}



int calcMoreBig(int valLum1, int valLum2, int adjustMin) {

  valLum1 = valLum1 - adjustMin;
  valLum2 = valLum2 - adjustMin;

  if (valLum1 >= valLum2) {
    return valLum1;
  } else {
    return valLum2;
  }
}
#include "RTClib.h"
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
#include <avr/power.h>  // Required for 16 MHz Adafruit Trinket
#endif


RTC_DS1307 rtc;


const int pinPRes_DX = A0;
const int pinPRes_SX = A1;
#define LED_PIN 6
#define LED_COUNT 420
#define switchA 7
#define switchB 8
#define switchC 9

struct Data {
  int year;
  int month;
  int day;
  char descri;
};

//Data birtDate[3];

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_RGB + NEO_KHZ800);

void setup() {

// ** NEOPIXEL
#if defined(__AVR_ATtiny85__) && (F_CPU == 16000000)
  clock_prescale_set(clock_div_1);
#endif
  strip.begin();             // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();              // Turn OFF all pixels ASAP
  strip.setBrightness(255);  // Set BRIGHTNESS to about 1/5 (max = 255)

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

  pinMode(switchA, INPUT);  // dichiariamo il pin del pulsante come ingresso
  pinMode(switchB, INPUT);  // dichiariamo il pin del pulsante come ingresso
  pinMode(switchC, INPUT);  // dichiariamo il pin del pulsante come ingresso
/*
  // carica l'array con le date da uscire in gay mode
  birtDate[0].day = 5;
  birtDate[0].month = 5;
  birtDate[0].year = 1993;
  birtDate[0].descri = "Compleanno_Enrico";

  birtDate[1].day = 14;
  birtDate[1].month = 5;
  birtDate[1].year = 1967;
  birtDate[1].descri = "Compleanno_Madre";

  birtDate[2].day = 31;
  birtDate[2].month = 1;
  birtDate[2].year = 1991;
  birtDate[2].descri = "Compleanno_Luca";

  birtDate[3].day = 26;
  birtDate[3].month = 8;
  birtDate[3].year = 1997;
  birtDate[3].descri = "Compleanno_Mic";

*/
  Serial.begin(9600);
}

// loop variable
int iHou;
int lum;
int pvS_A, pvS_B, pvS_C;
int S_A, S_B, S_C;
int SelectedMode = 0;
int count = 1;
int exIsec = 0;

uint32_t colorSec_1;
uint32_t colorMin_1;
uint32_t colorHour_1;

uint32_t colorSec_2;
uint32_t colorMin_2;
uint32_t colorHour_2;

uint32_t colorSec_3;
uint32_t colorMin_3;
uint32_t colorHour_3;

uint32_t colorSec_4;
uint32_t colorMin_4;
uint32_t colorHour_4;

uint32_t colorQuad_13;
uint32_t colorQuad_23;
uint32_t colorQuadAura;
bool gayMode;



void loop() {

  // leggo lo stato dei pulsanti
  S_A = digitalRead(switchA);  // -
  S_B = digitalRead(switchB);  // +
  S_C = digitalRead(switchC);  //serve per selezionare la modifica

  DateTime now = rtc.now();
  int iSec = now.second();
  int iMin = now.minute();
  int HourOrig = now.hour();
  iHou = Calc12H(HourOrig);

  // gestisco la luminosità totale
  lum = calcLum(pinPRes_DX, pinPRes_SX, iSec);

  gestioneIlluminazioneColore(lum, SelectedMode, iSec);

  // capisco se siamo in un giorno festivo
  //gayMode = verificaGiornoMese(iSec, iMin);
  //gayMode = false;

  // cancello tutti i led assegnati
  deleteAll();


  switch (SelectedMode) {
    case 0 ... 3:  // modalità normale

      switch (SelectedMode) {
        case 1 ... 3:  // modalità modifica
          adjustRTC(now);
          break;
      }

      //quandrante
      quadrante(colorQuad_13, colorQuad_23, colorQuadAura);

      // assegno i led per i secondi
      constrHand(colorSec_1, colorSec_2, colorSec_3, colorSec_4, iSec, 's', 0);

      // assegno i led per i minuti
      constrHand(colorMin_1, colorMin_2, colorMin_3, colorMin_4, iMin, 'm', 0);

      // assegno i led per le ore
      constrHand(colorHour_1, colorHour_2, colorHour_3, colorHour_4, iHou, 'h', iMin);

      strip.show();

      /* if (iSec == exIsec)
      {
        count = count + 1;

      }
      else
      {
        exIsec = iSec;
        count = 1;
      }; */

      break;

    case 4:  // modalità flash

      onAll();

      break;
  }

  if (S_C && !pvS_C) {

    SelectedMode = SelectedMode + 1;
    if (SelectedMode > 4) {
      SelectedMode = 0;
    }
  }

  pvS_A = S_A;
  pvS_B = S_B;
  pvS_C = S_C;
}



// *********** FUNZIONI ***********

// variable global
int calcMediaTOT;
int numCalcMedia;
int calcMedia;
int secSwitch_ex;
int secPastEX = 0;
uint16_t rainbowCycleSec = 0; // Variabile per far scorrere l'arcobaleno secondi
uint16_t rainbowCycleMin = 0; // Variabile per far scorrere l'arcobaleno minuti
uint16_t rainbowCycleOre = 0; // Variabile per far scorrere l'arcobaleno ore


bool verificaGiornoMese(int GiornoInt, int MeseInt){

  bool retBool = false;

/*   for (int i= 0; i < sizeof(birtDate); i++){

    if ( GiornoInt = birtDate.day){
      retBool = true;
    }


  } */

  return retBool;

}

void gestioneIlluminazioneColore(int lumiCalc, int typeMode, int secPast) {

  int lumiMaxCalc = 25;
  int lumiCalc_Aura;
  int lumiCalc_13;
  int lumiCalc_13Aura;
  int lumiCalc_23;
  int lumiRealCalc;
  int lumiRealCalcAuraQUAD;
  int lumiRealCalcQUAD_13;
  int lumiRealCalcQUAD_23;


  // faccio in modo che se la luminosità è troppo alta la aggiusti nella massima
  if (lumiCalc > lumiMaxCalc) {
    lumiCalc = lumiMaxCalc;
  }

lumiRealCalc = lumiCalc;

  // 1/3 calc
  lumiCalc_13 = lumiRealCalc / 3;
  lumiCalc_13Aura = lumiCalc_13 - 50;
  if (lumiCalc_13Aura < 1) lumiCalc_13Aura = 1;

  // 2/3 calc
  lumiCalc_23 = (lumiRealCalc / 3) * 2;
  if (lumiCalc_23 < 2) lumiCalc_23 = 2;

  // aura calc
  lumiCalc_Aura = lumiCalc / 2;
  if (lumiCalc_Aura < 3) lumiCalc_Aura = 3;


  // quad
  lumiRealCalcQUAD_13 = lumiCalc_23 / 6;
  lumiRealCalcQUAD_23 = lumiCalc_23 / 3;
  lumiRealCalcAuraQUAD = lumiRealCalc / 10;
  if (lumiRealCalcQUAD_13 < 2) lumiRealCalcQUAD_13 = 2;
  if (lumiRealCalcQUAD_23 < 3) lumiRealCalcQUAD_23 = 3;
  if (lumiRealCalcAuraQUAD < 1) lumiRealCalcAuraQUAD = 1;

  //if (gayMode){

/*     calcRainbowCycle(secPast);


    // devo differenziare il ciclo arcobaleno con quello normale
    // secondi
    colorSec_1 = Wheel(rainbowCycleSec, lumiCalc_13Aura, 2, lumiRealCalc);
    colorSec_2 = Wheel(rainbowCycleSec, lumiCalc_13, 2, lumiRealCalc);
    colorSec_3 = Wheel(rainbowCycleSec, lumiCalc_23, 1, lumiRealCalc);
    colorSec_4 = Wheel(rainbowCycleSec, lumiRealCalc, 0, lumiRealCalc);

     // minuti
    colorMin_1 =  Wheel(rainbowCycleMin, lumiCalc_13Aura, 2, lumiRealCalc);
    colorMin_2 =  Wheel(rainbowCycleMin, lumiCalc_13, 2, lumiRealCalc);
    colorMin_3 = Wheel(rainbowCycleMin, lumiCalc_13, 1, lumiRealCalc);
    colorMin_4 = Wheel(rainbowCycleMin, lumiRealCalc, 0, lumiRealCalc);
 
       // ore
    colorHour_1 = Wheel(rainbowCycleOre, lumiCalc_13Aura, 2, lumiRealCalc);
    colorHour_2 = Wheel(rainbowCycleOre, lumiCalc_23, 2, lumiRealCalc);  
    colorHour_3 = Wheel(rainbowCycleOre, lumiCalc_13, 1, lumiRealCalc);
    colorHour_4 = Wheel(rainbowCycleOre, lumiRealCalc, 0, lumiRealCalc);  */
 
  //}
  //else {

    //  * _1
    colorSec_1 = strip.Color(lumiCalc_Aura, lumiCalc_13Aura, lumiCalc_13Aura);
    colorMin_1 = strip.Color(lumiCalc_13Aura, lumiCalc_Aura, lumiCalc_13Aura);
    colorHour_1 = strip.Color(lumiCalc_13Aura, lumiCalc_13Aura, lumiCalc_Aura);

    //  * _2
    colorSec_2 = strip.Color(lumiRealCalc, lumiCalc_23, lumiCalc_23);
    colorMin_2 = strip.Color(lumiCalc_23,lumiRealCalc, lumiCalc_23);
    colorHour_2 = strip.Color(lumiCalc_23, lumiCalc_23, lumiRealCalc);

    //  * _3
    colorSec_3 = strip.Color(lumiRealCalc, lumiCalc_13, lumiCalc_13);
    colorMin_3 = strip.Color(lumiCalc_13, lumiRealCalc, lumiCalc_13);
    colorHour_3 = strip.Color(lumiCalc_13, lumiCalc_13, lumiRealCalc);

    //  * _4  // ore blu, minuti verdi, secondi rossi
    colorSec_4 = strip.Color(lumiRealCalc, 0, 0);
    colorMin_4 = strip.Color(0, lumiRealCalc, 0);
    colorHour_4 = strip.Color(0, 0, lumiRealCalc);
    // fine arcobaleno / normale

  //}

   // aura quad
  colorQuad_13 = strip.Color(lumiRealCalcQUAD_13, lumiRealCalcQUAD_13, lumiRealCalcQUAD_13);
  colorQuad_23 = strip.Color(lumiRealCalcQUAD_23, lumiRealCalcQUAD_23, lumiRealCalcQUAD_23);
  colorQuadAura = strip.Color(lumiRealCalcAuraQUAD, lumiRealCalcAuraQUAD, lumiRealCalcAuraQUAD);


  // se in modifica, revisiono i colori dei led in modifica
  switch (typeMode) {
    case 1:  //s
      colorSec_1 = strip.Color(255, 0, 255);
      colorSec_2 = strip.Color(255, 0, 255);
      colorSec_3 = strip.Color(255, 0, 255);
      colorSec_4 = strip.Color(255, 0, 255);
      break;
    case 2:  //m
      colorMin_1 = strip.Color(255, 0, 255);
      colorMin_2 = strip.Color(255, 0, 255);
      colorMin_3 = strip.Color(255, 0, 255);
      colorMin_4 = strip.Color(255, 0, 255);
      break;
    case 3:  //h
      colorHour_1 = strip.Color(255, 0, 255);
      colorHour_2 = strip.Color(255, 0, 255);
      colorHour_3 = strip.Color(255, 0, 255);
      colorHour_4 = strip.Color(255, 0, 255);
      break;
  }
  
}


void adjustRTC(DateTime time) {

  uint16_t yy;
  uint8_t mmm;
  int8_t dd;
  uint8_t hh;
  uint8_t mm;
  uint8_t ss;

  if ((S_B && !pvS_B) || (S_A && !pvS_A)) {  // se sto premento uno dei due pulsanti di cambio

    yy = time.year();
    mmm = time.month();
    dd = time.day();
    hh = time.hour();
    mm = time.minute();
    ss = time.second();

    if (S_B && !pvS_B) {
      switch (SelectedMode) {
        case 1:
          ss = ss - 1;
          if (ss <= 0) ss = 59;
          break;
        case 2:
          mm = mm - 1;
          if (mm <= 0) mm = 59;
          break;
        case 3:
          hh = hh - 1;
          if (hh <= 0) hh = 24;
          break;
      }
    } else if (S_A && !pvS_A) {
      switch (SelectedMode) {
        case 1:
          ss = ss + 1;
          if (ss >= 60) ss = 0;
          break;
        case 2:
          mm = mm + 1;
          if (mm >= 60) mm = 0;
          break;
        case 3:
          hh = hh + 1;
          if (hh >= 24) hh = 0;
          break;
      }
    }

    rtc.adjust(DateTime(yy, mmm, dd, hh, mm, ss));
  }
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

int calcLum(int pin1, int pin2, int secSwitch) {
  // serve per fare la media delle luminosità in entrata e quindi regolare di conseguenza la luminosità dell'orologio
  int calcMedia = 1;
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
    };

    if ((calcMedia + deltaLum) < calcMedia_prop || (calcMedia - deltaLum) > calcMedia_prop) {
      calcMedia = calcMedia_prop;
    };
    if (calcMedia <= 1) {
      calcMedia = 1;
    };
    Serial.println(calcMedia);
    // gestisco con il dato real
    // do una serie di regole per cui la luminosità sarà affiadata ad uno swith case
    switch (calcMedia) {
      case 1 ... 11:
        calcMediaFinal = 3;
        break;
      case 12 ... 30:
        calcMediaFinal = 6;
        break;
      case 31 ... 50:
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

int Calc12H(int HourOrig) {
  // serve per aggiustare le 24 ore
  int iHou;

  if (HourOrig > 12) {
    iHou = HourOrig - 12;
  } else {
    iHou = HourOrig;
  }
  return iHou;
}


int calcPosHour(int MinOrig) {

  int CompHour = 0;
  //faccio il compensatore per le ore
  if (MinOrig == 0 && MinOrig < 12) {
    CompHour = 0;
  } else if (MinOrig >= 12 && MinOrig < 24) {
    CompHour = 7;
  } else if (MinOrig >= 24 && MinOrig < 36) {
    CompHour = 14;
  } else if (MinOrig >= 36 && MinOrig < 48) {
    CompHour = 21;
  } else if (MinOrig >= 48 && MinOrig <= 59) {
    CompHour = 28;
  }
  return CompHour;
}



void constrHand(uint32_t color_1, uint32_t color_2, uint32_t color_3, uint32_t color_4, int nSector, char sType, int mCalc) {

  int ledStart;      // l'inizio della striscia a led
  int ledFinal;      // la fine della striscia a led da illuminare
  int ledLenght;     // la lunghezza della striscia a led da illuminare
  int ledCalc;       // i calcoli da fare per rappresentare i led
  int compHour = 0;  // per compensare le ore

  // in base a cosa voglio rappresentare, devo calcolare l'indirizzo di inizio

  switch (sType) {
    case 's':
      ledLenght = 6;
      ledCalc = 7;
      break;
    case 'm':
      ledLenght = 5;
      ledCalc = 7;
      break;
    case 'h':
      ledLenght = 4;
      compHour = calcPosHour(mCalc);
      ledCalc = 35;
      break;
  }

  // calcolo le posizioni dei led
  ledStart = (nSector * ledCalc) + compHour;
/*   if (ledStart > 419) {
    ledStart = ledStart - 420;
  } */
  ledStart = switchPixel420(ledStart);
  ledFinal = ledStart + ledLenght;


  strip.setPixelColor(ledStart, color_1);
  strip.setPixelColor(switchPixel420(ledStart + 1), color_2);
  strip.setPixelColor(switchPixel420(ledStart + 2), color_3);
  for (int i = switchPixel420(ledStart + 3); i < ledFinal; i++) {
    strip.setPixelColor(switchPixel420(i), color_4);
  }
 

 //arrowHand (ledFinal,color_2, color_3, color_4, sType);
 


}


void deleteAll() {
  for (int i = 0; i < strip.numPixels(); i++) {  // For each pixel in strip...
    strip.setPixelColor(i, strip.Color(0, 0, 0));
  }
}

void onAll() {
  for (int i = 0; i < strip.numPixels(); i++) {  // For each pixel in strip...
    strip.setPixelColor(i, strip.Color(255, 255, 255));
  }

  strip.setBrightness(255);
  strip.show();
  delay(100);
}

void quadrante(uint32_t color_13, uint32_t color_23, uint32_t colorAura) {

  for (int i = 0; i < 12; i++) {  // For each pixel in strip...
    strip.setPixelColor((i * 35) + 5, color_13);
    strip.setPixelColor((i * 35) + 6, colorAura);
  }

  for (int i = 0; i < 12; i += 3) {  // For each pixel in strip...
    strip.setPixelColor((i * 35) + 5, color_23);
  }
}






// Funzione per convertire una posizione sulla ruota dei colori in un colore RGB,
// con controllo per lo spostamento verso il bianco.
uint32_t Wheel(byte WheelPos, byte brightness, byte whiteShift, byte whiteShiftWhite) {
  WheelPos = 255 - WheelPos;
  byte red, green, blue;

  if (WheelPos < 85) {
    red = 255 - WheelPos * 3;
    green = 0;
    blue = WheelPos * 3;
  } else if (WheelPos < 170) {
    WheelPos -= 85;
    red = 0;
    green = WheelPos * 3;
    blue = 255 - WheelPos * 3;
  } else {
    WheelPos -= 170;
    red = WheelPos * 3;
    green = 255 - WheelPos * 3;
    blue = 0;
  }

  // Applica la luminosità a ciascuna componente RGB
  red = (red * brightness) / 255;
  green = (green * brightness) / 255;
  blue = (blue * brightness) / 255;

    if (red <= 0) red = 1;
    if (green <= 0) green = 1;
    if (blue <= 0) blue = 1;

  // Assicura che la luminosità sia almeno 1
  if (brightness < 1) {
    brightness = 1;
  }

  // Sposta verso il bianco
  if (whiteShift > 0) {
    byte whiteAmount = 0;
    if (whiteShift == 1) {
      whiteAmount = brightness / 3; // 1/3 della luminosità
    } else if (whiteShift == 2) {
      whiteAmount = (brightness * 2) / 3; // 2/3 della luminosità
    }

     if (whiteShiftWhite < 0){
        whiteShiftWhite = 1;
    }
    
    if (whiteShiftWhite > 0)
    {
       whiteAmount = (whiteAmount * whiteShiftWhite) / 255;
    }
    

    red = min(255, red + whiteAmount);
    green = min(255, green + whiteAmount);
    blue = min(255, blue + whiteAmount);
  }

    if (red <= 0) red = 1;
    if (green <= 0) green = 1;
    if (blue <= 0) blue = 1;

  return strip.Color(red, green, blue);
}

void arrowHand (int ledFinal, uint32_t color_2, uint32_t color_3, uint32_t color_4, char sType){

  uint32_t colorStamp_1;
  uint32_t colorStamp_2;

  switch (sType) {
    case 's':
      colorStamp_1 = color_4;
      colorStamp_2 = color_4;
      strip.setPixelColor(ledFinal - 3, strip.Color(0, 0, 0));
      strip.setPixelColor(ledFinal - 4, strip.Color(0, 0, 0));
      strip.setPixelColor(ledFinal - 5, strip.Color(0, 0, 0));
      break;
    case 'm':
      colorStamp_1 = color_3;
      colorStamp_2 = color_4;
        strip.setPixelColor(ledFinal - 3, strip.Color(0, 0, 0));
        strip.setPixelColor(ledFinal - 4, strip.Color(0, 0, 0));
      break;
    case 'h':
      colorStamp_1 = color_2;
      colorStamp_2 = color_3;
      strip.setPixelColor(ledFinal - 3, strip.Color(0, 0, 0));
      break;
  }

  //strip.setPixelColor(switchPixel420(ledFinal + 4), colorStamp_1);
  strip.setPixelColor(switchPixel420(ledFinal + 5), colorStamp_2);
  //strip.setPixelColor(switchPixel420(ledFinal + 11), colorStamp_1);
  strip.setPixelColor(switchPixel420(ledFinal - 9), colorStamp_2);
  //strip.setPixelColor(switchPixel420(ledFinal - 10), colorStamp_1);
  //strip.setPixelColor(switchPixel420(ledFinal - 17), colorStamp_1);

}

int switchPixel420 (int nPixel){
  int ledPixelOut = nPixel;

  // gestosto la lancetta che va oltre
  if (nPixel > 419) {
    ledPixelOut = nPixel - 420;
  }

  // gestisco la lancetta che va in negativo
  if(nPixel < 0){
    ledPixelOut = nPixel + 420;
  }
  return ledPixelOut;
}

void calcRainbowCycle(int secPast){
  // il ciclo parte una volta al secondo
   int intSum = 5;

  if (secPast != secPastEX){


    rainbowCycleSec = rainbowCycleSec + intSum;
    if (rainbowCycleSec >= 256 * 5) { // Fai un ciclo completo dell'arcobaleno
      rainbowCycleSec = 0;
    }
 
    rainbowCycleMin = rainbowCycleMin + intSum;
    if (rainbowCycleMin >= 256  * 5) { // Fai un ciclo completo dell'arcobaleno
      rainbowCycleMin = 0;
    }

    rainbowCycleOre = rainbowCycleOre + intSum;
    if (rainbowCycleOre >= 256 * 5) { // Fai un ciclo completo dell'arcobaleno
      rainbowCycleOre = 0;
    } 



    secPastEX = secPast;

  }

}



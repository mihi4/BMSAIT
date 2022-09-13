// settings and functions to drive LEDs through an MM5451 LED driver chip
// V0.1 13.09.2022

// datenfeld.target defines the MM5451 device (in an array)
// datenfeld.ref2 sets the pin of a single LED
// datenfeld.ref2 sets a timemark for fast blink (2 switches per second)
// datenfeld.ref3 sets a timemark for slow blink (1 switch per second)

#include "MM5451.h"
MM5451 mm5451[]={
    MM5451(2,3),
    MM5451(4,5)
}; // create instances of chip classes, parameters are CLK and DATA pin 

const byte mm5451anz = sizeof(mm5451)/sizeof(mm5451[0]); 

void SetupLED_MM5451()
{
    for (int x=0; x<mm5451anz; x++) {
        mm5451[x].lightAll();
    }
    delay(800);
    for (x=0; x<mm5451anz; x++) {
        mm5451[x].clearAll();
    }
}


void LED_On(byte chipIndex, byte ledNumber) {  // this function sets the status of the appropriate output of the chip, but doesn't send it yet
    mm5451[chipIndex].setOutput(ledNumber, true);
}
void LED_Off(byte chipIndex, byte ledNumber) {
    mm5451[chipIndex].setOutput(ledNumber, false);
}

void outputData(byte chipIndex) {
    mm5451[chipIndex].outputData();
}

void UpdateLED_MM5451(byte p)
{
  // if the first character is T(rue) or 1 (on, no blink), the LED will be turned on
  if ((datenfeld[p].wert[0]=='T') || (datenfeld[p].wert[0]=='1'))
  {
    LED_On(datenfeld[p].target,datenfeld[p].ref2);
  }
  else if (datenfeld[p].wert[0]=='3') //fast blinking LED
  {
    if (datenfeld[p].ref3==1)
    { LED_On(datenfeld[p].target,datenfeld[p].ref2); }
    else
    { LED_Off(datenfeld[p].target,datenfeld[p].ref2); }
  }
  else if (datenfeld[p].wert[0]=='2') //slow blinking LED
  {
    if (datenfeld[p].ref4==1)
    { LED_On(datenfeld[p].target,datenfeld[p].ref2); }
    else
    { LED_Off(datenfeld[p].target,datenfeld[p].ref2);}
  }
  else         // otherwise the LED will be turned off
  {
    LED_Off(datenfeld[p].target,datenfeld[p].ref2);
  }
  
  mm5451[(datenfeld[p].target)].outputDataBits();  // let MM5451 chip send out all LED data bits
  
  if (millis()>LEDTimer+BLINKSPEED)
  { UpdateBlink(); }
}

/*

void SetupLEDMatrix()
{
  delay(1000);
  LEDM[0]=LedControl(LEDM_DIN, LEDM_CLK, LEDM_CS, 1);  //actual call of the LEDControl class with the correct PINs
  LEDM[0].shutdown(0,false);
  LEDM[0].setIntensity(0,LEDM_BRIGHTNESS);
  LEDM[0].clearDisplay(0);

  //LEDM[1]=LedControl(0/*DIN*/,0 /*CLK*/,0 /*LEDM_CS*/, 1);  //actual call of second LEDControl class with the correct PINs
  //LEDM[1].shutdown(0,false);
  //LEDM[1].setIntensity(0,LEDM_BRIGHTNESS);
  //LEDM[1].clearDisplay(0);
/*
  delay(1000);
}

void UpdateLEDMatrix(byte p)
{
  if ((datenfeld[p].wert[0]=='T'))        // if the first character is T(rue), the LED will be turned on
  {
    LEDM[datenfeld[p].target].setLed(0,datenfeld[p].ref2,datenfeld[p].ref3,true);
  }
  else                                  // otherwise the LED will be turned off
  {
    LEDM[datenfeld[p].target].setLed(0,datenfeld[p].ref2,datenfeld[p].ref3,false);
  }
}

// settings and functions to drive simple LED
// V1.3.7 26.09.2021

// datenfeld.target defines the PIN of a single LED
// datenfeld.ref2 sets LED brightness (0..255) -->works on pwm PINs only!
// datenfeld.ref2 sets a timemark for fast blink (2 switches per second)
// datenfeld.ref3 sets a timemark for slow blink (1 switch per second)
#ifdef ESP
  #include <analogWrite.h>
#endif

#define BLINKSPEED 500  //pause (in ms) between on/off for fast blinking. Slow blinking will be 50%

unsigned long LEDTimer;

void SetupLED()
{
   //all LED PINs are set to output
  for(byte a = 0; a < VARIABLENANZAHL; a++)
  {
    if (datenfeld[a].typ==10)
    {
      pinMode(datenfeld[a].target, OUTPUT);
      digitalWrite(datenfeld[a].target,LOW);
    }
    if (datenfeld[a].typ==11)
    {
      pinMode(datenfeld[a].target, OUTPUT);
      digitalWrite(datenfeld[a].target,HIGH);
    }
  }
  LEDTimer=millis();;
}

void UpdateBlink()
{
  LEDTimer=millis();
  for(byte a = 0; a < VARIABLENANZAHL; a++)
  {
    if ((datenfeld[a].typ==10) || (datenfeld[a].typ==11))
    {
      if (datenfeld[a].ref3==1)
      {
        datenfeld[a].ref3=0;
        if (datenfeld[a].ref4==0)
          {datenfeld[a].ref4=1;}
        else
          {datenfeld[a].ref4=0;}
      }
      else
      {
        datenfeld[a].ref3=1;
      }
    }
  }
}


void LED_On(byte pIN, byte brightness, boolean currDir)
{
  if (brightness>0) //set brightness (only works on pwm PINs)
  {
    if (currDir)
      analogWrite(pIN,brightness);
    else
      analogWrite(pIN,255-brightness);
  }
  else
  {
    digitalWrite(pIN,currDir);
  }
}

void LED_Off(byte pIN, boolean currDir)
{
  digitalWrite(pIN,!currDir);
}

void UpdateLED(byte p, boolean currDir)
{
  // if the first character is T(rue) or 1 (on, no blink), the LED will be turned on
  if ((datenfeld[p].wert[0]=='T') || (datenfeld[p].wert[0]=='1'))
  {
    LED_On(datenfeld[p].target,datenfeld[p].ref2,currDir);
  }
  else if (datenfeld[p].wert[0]=='3') //fast blinking LED
  {
    if (datenfeld[p].ref3==1)
    { LED_On(datenfeld[p].target,datenfeld[p].ref2,currDir); }
    else
    { LED_Off(datenfeld[p].target,currDir); }
  }
  else if (datenfeld[p].wert[0]=='2') //slow blinking LED
  {
    if (datenfeld[p].ref4==1)
    { LED_On(datenfeld[p].target,datenfeld[p].ref2,currDir); }
    else
    { LED_Off(datenfeld[p].target,currDir);}
  }
  else         // otherwise the LED will be turned off
  {
    LED_Off(datenfeld[p].target,currDir);
  }

  if (millis()>LEDTimer+BLINKSPEED)
  { UpdateBlink(); }
}

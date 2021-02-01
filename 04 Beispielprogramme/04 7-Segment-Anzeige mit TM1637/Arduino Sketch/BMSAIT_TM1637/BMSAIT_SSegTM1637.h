
// This LEDDisplayDriver.h library is used to drive a TM1637 module with 6 digits
// Manual for library: http://lygte-info.dk/project/DisplayDriver%20UK.html
// By HKJ from lygte-info.dk

// settings and functions to display numbers on 7-segment-displays
//target= device ID (if multiple displays are connected
//ref2= not used
//ref3= number of figures to display
//ref4= offset position 
//ref5= decimal point

#include <LEDDisplayDriver.h>

#ifndef _TM1637_
#error "_TM1637_ must be defined in LEDDisplayDriver.h for this sketch to work"
#endif

// Call 7-Segment-Display with TM1637 controller
#define TM1637_SDA_PIN 2 //DIO
#define TM1637_SCL_PIN 3 //CLK
#define TM1637_BRIGHTNESS 0x0f

LEDDisplayDriver TM1637_display[] = {
    LEDDisplayDriver(TM1637_SDA_PIN, TM1637_SCL_PIN,true, 6)
    //LCDDisplayDriver(/*SDA_PIN*/,/* SCL_PIN*/,true, /*num of digits*/)
};


void SetupTM1637()
{
  TM1637_display[0].setBrightness(TM1637_BRIGHTNESS);
  TM1637_display[0].clear();
  //TM1637_display[1].setBrightness(TM1637_BRIGHTNESS);
  //TM1637_display[1].clear();
}


void UpdateTM1637(int p)
{
  byte pos=0;
  byte decimal;
  for (byte x=0;x<datenfeld[p].ref3;x++)
  {
    switch (5-x-datenfeld[p].ref4)
    {
      case 0: 
        pos=3; 
        break;
      case 1: 
        pos=4; 
        break;
      case 2: 
        pos=5; 
        break;
      case 3: 
        pos=0; 
        break;
      case 4: 
        pos=1; 
        break;
      case 5: 
        pos=2; 
        break;
    }
    if ((datenfeld[p].ref5-1)==x)
      TM1637_display[datenfeld[p].target].showNumWithPoint(datenfeld[p].wert[x]-48,0,pos,1);
    else 
      TM1637_display[datenfeld[p].target].showNum(datenfeld[p].wert[x]-48,pos,1);
  }
}

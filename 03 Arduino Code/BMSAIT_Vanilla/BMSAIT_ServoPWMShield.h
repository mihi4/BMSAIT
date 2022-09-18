// settings and functions to drive servo motors (via pwm motor shield)
// V1.3.7 26.09.2021

//target= reference link to the line of the servodataPWM table of this module
//ref2= not used
//ref3= not used
//ref4= not used
//ref5= not used

#include <Adafruit_PWMServoDriver.h>

typedef struct
{
byte port;          //channel of the pwm shield the motor is connected to
int minPulse;       //min pulse for the motor 
int maxPulse;       //max pulse for the motor 
int a_ug;           //min absolut value that might be displayed (i.e.   0 for a speed indicator)
int a_og;           //max absolut value that might be displayed (i.e. 600 for a speed indicator)
int last;           //previous value
unsigned long lu;   //last update
} ServodataPWM;

//IMPORTANT:  You need to calibrate each servo to prevent damage.
//during calibration, each servo will be moved to the min/max value and stay there for 1 second. If the servo is vibrating in a min or max position, the position is out of bounds. 
//in this case, you need to increase the min value (minPulse) or reduce the max value (maxPulse) in the following table until the servo does not vibrate in the min/max positions

// if you are using a float variable, multiply a_ug and a_og with 100 to get better resolution!!

ServodataPWM servodataPWM[] =
{
// Channel  minPulse   maxPulse   a_ug   a_og   last  lu
    {0,       120,       620,      -32768,    32767,     0,   0},  // RPM   
	{1,       120,       620,      -32768,    32767,     0,   0}  // nozzPos
};
const int servozahlPWM = sizeof(servodataPWM)/sizeof(servodataPWM[0]);

int longVars[] = { 201,251,261,281,291,301,311,321,331,341,351,481,531 };  // array of var numbers that get send as 0-65535 (too big for a_og/a_ug as ints)
const int longVarsNum = sizeof(longVars)/sizeof(longVars[0]);

#define SERVODELAYPWM 5
#define SERVOSLEEPDELAY 40000

bool pwmSleeping = false;

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver( 0x40 );


void SetupServoPWM()
{  
  pwm.begin();
  pwm.setPWMFreq(60);

  for (byte x=0;x<servozahlPWM;x++) //move all servos to center position
  {  
    int pulselength = map(90, 0, 180, servodataPWM[x].minPulse, servodataPWM[x].maxPulse);
    pwm.setPWM(x, 0, pulselength);
    delay(10);
  }
  delay(500);  
}

void ServoPWM_FastUpdate()
{}

void ServoPWM_Zeroize(bool mode)
{
  for (byte x=0;x<servozahlPWM;x++) //move all servos to min position
  {  
    int pulselength = map(0, 0, 180, servodataPWM[x].minPulse, servodataPWM[x].maxPulse);
    pwm.setPWM(x, 0, pulselength);
    delay(10);
  }
  if (mode)
  {
    delay(1000);
    for (byte x=0;x<servozahlPWM;x++) //move all servos to max position
    {  
      int pulselength = map(180, 0, 180, servodataPWM[x].minPulse, servodataPWM[x].maxPulse);
      pwm.setPWM(x, 0, pulselength);
      delay(10);
    }
    delay(1000);
    for (byte x=0;x<servozahlPWM;x++) //move all servos to center position
    {  
      int pulselength = map(90, 0, 180, servodataPWM[x].minPulse, servodataPWM[x].maxPulse);
      pwm.setPWM(x, 0, pulselength);
      delay(10);
    }
  }
}

void UpdateServoPWM(int d)
{
  byte servoID=datenfeld[d].target;

  if (servodataPWM[servoID].lu+SERVOSLEEPDELAY<millis() && (!pwmSleeping))
  {
    pwm.sleep();         //set servo to sleep mode if no new signal arrived for SERVOSLEEPDELAY milliseconds
	pwmSleeping = true;
    if (debugmode){SendMessage("PWM went to sleep",1);}
  }  
    
  if (servodataPWM[servoID].lu + SERVODELAYPWM < millis())
  {

	long newValLong;  // changed to long, since the gauge vars go from 0 to 65535
	if (datenfeld[d].format == 'f') { // check, if data received is a float value; if yes, multiply by 100
		newValLong = atof(datenfeld[d].wert)*100;	 
	} else {
		newValLong = atol(datenfeld[d].wert);
	}	
	
	int newValInt;
	// convert uint 0-65535 to int -32768 - 32767
	int checkNum = atoi(datenfeld[d].ID);
	bool convertFromLong = false;
	for (int x=0; x<longVarsNum; x++) {
		if (checkNum == longVars[x]) { convertFromLong = true; }
	}	
	if ( convertFromLong ) {				
		newValInt = newValLong-32768;
	} else {
		newValInt = newValLong;
	}
	
    if (servodataPWM[servoID].last != newValInt)
    {
      if (pwmSleeping) { //wake up servo
		pwm.wakeup(); 
		pwmSleeping = false; 
		SendMessage("wakeup PWM",1);
	  } 
      servodataPWM[servoID].lu = millis();
      servodataPWM[servoID].last = newValInt; 
		if (debugmode) {			
			SendMessage("servo Update: ",1);
			SendMessage(datenfeld[d].ID,1);
			SendMessage((datenfeld[d].wert),1);
			SendMessage("newValLong: ",1);
			char buffer[30]; 
			SendMessage(ltoa(newValLong, buffer, 10),1);
			SendMessage("newValInt: ",1);
			SendMessage(itoa(newValInt, buffer, 10),1);
		} 
  
      //calculate pulselength 
      uint16_t pulselength=0;
      if (servodataPWM[servoID].last<servodataPWM[servoID].a_ug)
        {pulselength=servodataPWM[servoID].minPulse;}
      else if (servodataPWM[servoID].last>servodataPWM[servoID].a_og)
        {pulselength=servodataPWM[servoID].maxPulse;}
      else
        {pulselength = map(servodataPWM[servoID].last, servodataPWM[servoID].a_ug, servodataPWM[servoID].a_og, servodataPWM[servoID].minPulse, servodataPWM[servoID].maxPulse);}
      
      //move servo
      pwm.setPWM(servodataPWM[servoID].port, 0, pulselength);         
    }
  }
}

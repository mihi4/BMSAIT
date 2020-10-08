// Arduino sketch to send/recieve data from the Falcon BMS Shared Memory via the BMS-Arduino Interface Tool and control devices in home cockpits
// Version: 1.0    3.10.2020
// Robin "Hummer" Bruns


//basic settings
  #include <Arduino.h>

  #define DATENLAENGE 8       // maximum length of a data set
  #define BAUDRATE 57600      // serial connection speed
  #define MESSAGEBEGIN 255    // this byte marks the beginning of a new message from the Windows app
  #define HANDSHAKE 128       // this byte marks an identification request from the Windows app
  #define CALIBRATE 160       // this byte marks a request to reset motors to inital position
  #define STARTPULL 170       // this byte marks a request to start the PULL logic on the arduino
  #define ENDPULL 180         // this byte marks a request to end the PULL logic on the arduino
  #define TESTON  190         // activates testmode
  #define TESTOFF 200         // deactivates testmode
  #define VAR_BEGIN '<'       // char to mark the begin of the actual data
  #define VAR_ENDE '>'        // char to mark the end of the actual data
  #define TYP_ANFANG '{'      // char to mark the begin of the type definition in a message string
  #define TYP_ENDE '}'        // char to mark the begin of the type definition in a message string
  
//Function headers
  void PullRequest();
  void ReadResponse();     
  void debug_readback(byte posID);
  void SendSysCommand(char text[]); 
  void SendMessage(char message[], byte option);
  void ResetMotors();
  //mod
  void debug_readbackBUP(byte posID);
  void CheckSwitchesBUPRadio(byte index);
  //mod
  
//Struct definitions
  typedef struct //data field structure for storage of data variables
  {
    char bezeichnung[6];            //short description (max. 5 characters)
    char ID[5];                     //only required for PULL-operation. The ID for this data from "BMSAIT-Variablen.csv". this will tell the windows app which data you are requesting
    char format;                    //only required for PULL-operation. Data type according to  "BMSAIT-Variablen.csv" (y=byte, i=integer, f=float, s=string, b=bool, 1=Byte Array, 2=Int Array, 3=string Array, 4=float Array) 
    byte typ;                        //indicates the type of device that will be used to output his data (10: LED, 20: LCD, 30: 7-Segment MAX7219, 40: Servomotor)
    byte ziel;                       //target information. This will be used to set the PIN for LED output, the adress for motors on a pwm shield or the line on LCD displays
    byte stellen;                    //sets the length of this data. This is used to set the position of this data on LCD or 7-segment displays
    byte start;                      //sets the position of this data on LCD / 7Segment displays. if set, the data will be offset by this number of characters
    byte dp;                         //marks the character that will display the decimal point on 7Segment displays
    char wert[DATENLAENGE];         //contains the current value
  } Datenfeld;

  typedef struct              //container to store incoming data from the Windows app
  {
    byte varNr;                      //position for this data in the local file container 
    char typ;                       //file format of this data 
    char wert[DATENLAENGE];         //data
  } Uebertragung;
  Uebertragung neuer_wert;

//Load user settings
  #include "BMSAIT_UserConfig.h"   //load the user settings 

//Global variables
  byte inputByte_0;           //container for incoming serial message
  byte inputByte_1;           //container for incoming serial message
  byte state =0;              //marker to memorize the current position in a message string
  byte Uebertragung_pos=0;    //counts how many chars have already been read from an incoming data stram. Used to prevent an overflow of data variables. 
  bool testmode=false;        //Testmode on/off

//settings for PULL logic
//  bool pull= false;           //flag that defines wether to use push or pull logic (true=pull ; false=push). Can be switched by commands of the F4SMExporter App
//  long lastPoll=0;            //timestamp of the last data request during pull logic
//  byte last_request=99;        //marker for the last variable that was requested via pull logic in the last loop 
//settings for PULL logic


//mod

//switch and signal settings
#ifdef Switches                
  #include "BMSAIT_Switches.h" 
#endif  

#ifdef BUPRadio              
  #include "BMSAIT_BUPRadio.h"
#endif 
//mod  

//LED settings
#ifdef LED                             
  #include "BMSAIT_LED.h"
#endif

//LCD settings
#ifdef LCD                              
  #include "BMSAIT_LCD.h"
#endif    

//7Segment settings MAX 7219 tube
#ifdef SSegMAX7219                              
  #include "BMSAIT_SSegMAX7219.h"
#endif                                

//7Segment settings TM1637 tube
#ifdef SSegTM1637                            
  #include "BMSAIT_SSegTM1637.h"
#endif  

//Servo settings
#ifdef ServoMotor                       
  #include "BMSAIT_Servo.h"               
#endif                            

//PWM Shield for Servos settings
#ifdef ServoPWM                       
  #include "BMSAIT_ServoPWMShield.h"               
#endif  

//Steppermotor settings
#ifdef StepperBYJ                  
  #include "BMSAIT_Stepper28BYJ48.h"
#endif                                

//Steppermotor settings
#ifdef StepperX27                 
  #include "BMSAIT_StepperX27.h"
#endif  

//Steppermotor ControllerBoard settings
#ifdef StepperVID                   
  #include "BMSAIT_StepperVID.h"
#endif  

//motor poti settings
#ifdef MotorPoti 
  #include "BMSAIT_MotorPoti.h"
#endif 

 

//button matrix settings
#ifdef ButtonMatrix               
  #include "BMSAIT_Buttonmatrix.h"
#endif  

//rotary encoders
#ifdef RotEncoder               
  #include "BMSAIT_Encoder.h"
#endif  

//analog axis settings
#ifdef AnalogAxis 
  #include "BMSAIT_Analogachse.h"
#endif 
  
//example how to add your own project to this sketch. 
#ifdef NewDevice                
  #include "BMSAIT_Placeholder.h"
#endif  
  
 

void setup()
{
  Serial.begin(BAUDRATE);
  pinMode(LED_BUILTIN, OUTPUT);
                                      
  #ifdef LED                            //LEDs setup begin
    SetupLED();
  #endif                                //LEDs setup end
  
  #ifdef LCD                            //LCD setup begin
   SetupLCD();
  #endif                                //LCD setup end
  
  #ifdef SSegMAX7219                    //7-segment-display MAX7219 setup begin
   SetupMax7219();
  #endif                                //7-segment-display MAX7219 setup end

  #ifdef SSegTM1637                    //7-segment-display TM1637 setup begin
   SetupTM1637();
  #endif                                //7-segment-display TM1637 setup end
                             
  #ifdef ServoMotor                     //servo setup begin
   SetupServo();
  #endif                                //servo setup end
  
  #ifdef ServoPWM                       //servo PWM shield setup begin
    SetupServoPWM();          
  #endif                                //servo PWM shield setup end
    
  #ifdef StepperBYJ                     //stepper setup begin
   SetupStepperBYJ();
  #endif                                //stepper setup end

  #ifdef StepperX27                     //stepper setup begin
   SetupStepperX27();
  #endif                                //stepper setup end
  
  #ifdef StepperVID                     //stepper on controller board setup begin
   SetupStepperVID();
  #endif                                //stepper setup end
  
  #ifdef MotorPoti                      //MotorPoti setup begin
    SetupMotorPoti();
  #endif                                //MotorPoti setup end
  #ifdef Switches                       //Input controller setup begin
   SetupSwitches();
  #endif                                //Input controller setup end
  
  #ifdef ButtonMatrix                   //Buttonmatrix setup begin
   SetupButtonMatrix();
  #endif                                //Buttonmatrix setup end
  
  #ifdef RotEncoder                     //Rotary encoder setup begin
   SetupEncoder();
  #endif                                //Rotary encoder setup end

  #ifdef AnalogAxis                     //Analog axis setup begin
   SetupAnalog();
  #endif                                //Analog axis setup end  
  
  #ifdef NewDevice                      //Example. Use this placeholder to activate your own projects
    SetupNewDevice();
  #endif                                //Example end

  //modification for BUPRadio
  #ifdef BUPRadio 
    SetupBUPRadio();
  #endif
  //modification for BUPRadio
}

      

/// Main loop
void loop()              
{
  delay(1);
  
  //if (pull){PullRequest(); }   //send data requests if PULL mode is active
  
  ReadResponse();              //check for new data from the windows app

  #ifdef Switches              
   CheckSwitches();            //check switch positions and initiate commands if switches were moved
  #endif
                                 
  #ifdef ButtonMatrix         
   ButtonmatrixRead();         //check button matrix for changes and initiate commands if switches were moved 
  #endif
  
  #ifdef RotEncoder              
   CheckEncoder();            //check encoders and initiate commands if they were moved
  #endif
  
  #ifdef AnalogAxis      
   ReadAnalogAxis();           //check analog axis for changes and initiate commands if switches were moved
  #endif
  
  for (byte x=0;x<VARIABLENANZAHL;x++)     //loop through the data container. For each entry, the respective output will be updated
  {
    switch (datenfeld[x].typ)
    {
      #ifdef LED
        case 10: //LED (PIN is wired to GND)
          UpdateLED_PINHIGH(x); 
          break;
        case 11: //LED (PIN is wired to Vcc)
          UpdateLED_PINLOW(x); 
          break;
      #endif       
      
      #ifdef LCD
        case 20: //LCD
          Update_LCD(x);
          break;
      #endif
        
      #ifdef SSegMAX7219 
        case 30:  //7-Segment display Max7219
          UpdateMAX7219(x);
          break;
      #endif
      
      #ifdef SSegTM1637
        case 31:   //7-Segment display TM1637
          UpdateTM1637(x);
          break;
      #endif
      
      #ifdef ServoMotor
        case 40: //standard Servos (i.e. SG90)
          UpdateServo(x);
          break;
      #endif
      
      #ifdef ServoPWM  
        case 41: //Servo on PWM Shield
          UpdateServoPWM(x);
          break;
      #endif
      
      #ifdef StepperBYJ 
        case 50: //Stepper Motor
          UpdateStepperBYJ(x);
          break;
      #endif    
   
      #ifdef StepperX27
        case 51: //X27.168 directly connected to the arduino          
          UpdateStepperX27(x);
          break;
      #endif
    
      #ifdef StepperVID
        case 52: //one to four Servos on a VID6606 board
          UpdateStepperVID(x);
          break;
      #endif
      
      #ifdef MotorPoti
        case 60: //MotorPoti
          UpdateMotorPoti(x);
          break;      
      #endif
      
      #ifdef NewDevice  //define this flag in the top of BMSAIT_UserConfig.h to activate this block ("#define newDevice")
        case 69: //assign this type to a variable in the data container to call a new method
          UpdateNewDevice(x);  //program a new method void Update_newDevice(int p){command1;command2;...}to enable your device 
        break;
      #endif

     
      default:
        //not implemented
        break;
    }
  }
}


///Loop through the data container and send a message to the BMSAIT App to request data update for each entry
void PullRequest()
{
/*
  if (lastPoll+POLLTIME<millis())  //wait for a specified time (POLLTIME) before requesting another update
  {
    lastPoll=millis();
    while (Serial.available())  //flush input buffer to prevent wrong data to be read into the current variable
    {Serial.read();}      
    
    //in every loop run, only one data entry update will be requested. This avoids data congestion and flickering of attached displays
    if (last_request>=VARIABLENANZAHL-1)
      {last_request=0;}
    else
      {last_request++;}
    
    //build message string <pos>,<vartype>,<varID>
    char nachricht[11]="";
    char pos[2]="";
  if (strcmp(datenfeld[last_request].ID,"9999")==0) return;  //don't update dummy variables                                            
    itoa(last_request,pos,10);  //write data container position as character
    if (last_request<10) 
      {
        nachricht[0]='0'; 
        nachricht[1]=pos[0];
      }
      else 
      {
        nachricht[0]=pos[0]; 
        nachricht[1]=pos[1];
      }
    nachricht[2]=',';
    nachricht[3]=datenfeld[last_request].format;  //add the variable type
    nachricht[4]=',';
    for (byte lauf=0;lauf<5;lauf++)
    {nachricht[5+lauf]=datenfeld[last_request].ID[lauf];} //add the variable ID
    nachricht[10]='\0';
    SendMessage(nachricht,2);
  //modification BUP Radio
    //delay(10);
    delay(5);
    PullRequestBUP();  //adds another request to update the BUPFreq container
  //modification BUP Radio
  }
*/
}

/// reset stepper motor positions to default values
void ResetMotors()
{
#ifdef ServoMotor                       
  Servo_Zeroize();            
#endif                            

#ifdef ServoPWM                       
  ServoPWM_Zeroize();              
#endif  

#ifdef StepperVID
  StepperVID_Zeroize();   
#endif
 
#ifdef StepperX27
  StepperX27_Zeroize();  
#endif
 
#ifdef Stepper28BYJ48
  Stepper28BYJ48_Zeroize();  
#endif
 
#ifdef MotorPoti
  MotorPoti_Zeroize(); 
#endif
}

/// Check incoming serial data for data. Expects structured messages --> CommandBit VariableID {VariableType}<Data>
void ReadResponse()       
{
  if (Serial.available()>1)
  {
     if (state==0)
     {
       inputByte_0=Serial.read();
       if (inputByte_0 == MESSAGEBEGIN ) //Check for start of Message - byte (255)
       {
         state=1;
       }
     }
     
     if (state==1)
     {
       inputByte_1=Serial.read();
       if (inputByte_1 == HANDSHAKE)
       {
         Serial.flush();
         SendSysCommand(ID); //Send ID to idendify this board
         delay(1000);
         while (Serial.available()){Serial.read();} //clear buffer
         state=0;
       }
       else if (inputByte_1 == STARTPULL)
       {
         //confirm start of PULL requests
         //pull=true;
         Serial.flush();
         SendSysCommand("on");
         delay(1000);
         while (Serial.available()){Serial.read();} //clear buffer
         state=0;
       }  
       else if (inputByte_1 == ENDPULL)
       {
         //confirm termination of PULL requests
         //pull=false;
         Serial.flush();
         SendSysCommand("off");
         delay(1000);
         while (Serial.available()){Serial.read();} //clear buffer
         state=0;
       } 
       else if (inputByte_1 == CALIBRATE)
       {
         //reset motor position to zero
         Serial.flush();
         SendSysCommand("cal");
         delay(1000);
         while (Serial.available()){Serial.read();} //clear buffer
         ResetMotors();
         state=0;
       }       
       else if (inputByte_1 == TESTON)
       {
         //confirm testmode
         testmode=true;
         Serial.flush();
         SendSysCommand("on");
         delay(1000);
         while (Serial.available()){Serial.read();} //clear buffer
         state=0;
       }  
       else if (inputByte_1 == TESTOFF)
       {
         //confirm end of testmode
         testmode=false;
         Serial.flush();
         SendSysCommand("off");
         delay(1000);
         while (Serial.available()){Serial.read();} //clear buffer
         state=0;
       }
             
       else if (((int)inputByte_1 < VARIABLENANZAHL) || ((int)inputByte_1 >100)) //check if ID is valid
       {
         neuer_wert.varNr=(int)inputByte_1;
         state=2;
       }
       else 
       { 
         state=0;    //unexpected value. discard data and start over  
         if (testmode){SendMessage("Fehler State 1",1);}
       }
     }

     if ((state==2) && (Serial.available()>2))
      {
        if (Serial.read()==TYP_ANFANG) //if the message is in the correct format, the defined char TYP_ANFANG can be found here
        {
          neuer_wert.typ=Serial.read();  //reads the VariableType ('i'=integer, 'f'=float, 's'=string ...)
          Serial.read();  //Dump the char '}'
          state=3;
        }
        else
        {
          state=0;  //unexpected value. discard data and start over
          if (testmode){SendMessage("Fehler State 2",1);}
        }  
      }
      
      if ((state==3) && (Serial.available()>1))
      {
        if (Serial.read()==VAR_BEGIN) //if the message is in the correct format, the defined char VAR_ANFANG can be found here
        {
          state=4;
        }
        else
        {
          state=0;  //unexpected value. discard data and start over
          if (testmode){SendMessage("Fehler State 3",1);}
        }  
      }
      
    while ((state==4) && (Serial.available()))
    {
      char c = Serial.read();
      if ((c==VAR_ENDE) || (Uebertragung_pos>DATENLAENGE-1))  //continue to read the incoming data until the termination character is read or the maximum lengh of the data variable is reached
      {
        // end of data found. Validate the buffer before writing the new data into the data container
        state=0;
        int laenge=sizeof(neuer_wert.wert);            
        if (neuer_wert.varNr<99)     //only compute the data if a valid data position is found (everything above 99 is invalid)
        {
          if (strcmp(datenfeld[neuer_wert.varNr].wert, neuer_wert.wert)!=0)  //check if the recieved data is different from the stored data
          {
             for (int lauf=DATENLAENGE;lauf>=laenge;lauf--)
               {datenfeld[neuer_wert.varNr].wert[lauf]='\0';}
             memcpy(datenfeld[neuer_wert.varNr].wert, neuer_wert.wert, sizeof(neuer_wert.wert)); //write the new data into the data container
             if (testmode){debug_readback(neuer_wert.varNr);}
          }
        }
        //modification for BUP Radio (varNr is between 101 and 121)
        else 
        {
          for (byte lauf=DATENLAENGE-1;lauf>laenge;lauf--)
            {BUPRadioFreq[neuer_wert.varNr-100].wert[lauf]='\0';}
            
          memcpy(BUPRadioFreq[neuer_wert.varNr-100].wert, neuer_wert.wert, sizeof(neuer_wert.wert)); //write the new data into the data container
          //debug 
            if (neuer_wert.varNr==120)
            for (byte x=101;x<121;x++)
            {
              debug_readbackBUP(x-100); //send current data back to F4SME App for debug purposes
              delay(10);
            }
          //debug  
        }
        //modification for BUP Radio
        
        //clean variables
        Uebertragung_pos=0;
        neuer_wert.wert[0]="\0";
        neuer_wert.varNr=-1;
        neuer_wert.typ='\0';
      }
      else                
      {
        //end of data not yet found. Add the current character to the buffer.
        neuer_wert.wert[Uebertragung_pos]=c;
        neuer_wert.wert[Uebertragung_pos + 1]='\0';
        Uebertragung_pos++;
      }
    }
  }
}

//readback of recieved data for verification
void debug_readback(byte posID)
{
  byte laenge=sizeof(datenfeld[posID].wert);
  char antwort[laenge+2]="";
  char pos[3]="";
  itoa(posID,pos,10);
  if (posID<10)
  {
    antwort[0]=pos[0];
    antwort[1]=' ';
    antwort[2]=' ';
  }
  else
  {
    antwort[0]=pos[0];
    antwort[1]=pos[1];
    antwort[2]=' ';
  }
  for (int lauf=0;lauf<laenge;lauf++)
    {antwort[lauf+3]=datenfeld[posID].wert[lauf];}
  antwort[laenge+2]='\0';
  SendMessage(antwort,1);
}

void debug_readbackBUP(byte posID)
{
  byte laenge=sizeof(BUPRadioFreq[posID].wert);
  char antwort[laenge+4]="";
  char pos[4]="";
  itoa(posID,pos,10);
  if (posID<10)
  {
    antwort[0]=pos[0];
    antwort[1]=' ';
    antwort[2]=' ';
  }
  else
  {
    antwort[0]=pos[0];
    antwort[1]=pos[1];
    antwort[2]=' ';
  }
  for (byte lauf=0;lauf<laenge;lauf++)
    {antwort[lauf+3]=BUPRadioFreq[posID].wert[lauf];}
  antwort[laenge+3]='\0';
  SendMessage(antwort,1);
}

///send a system command to the BMSAIT App 
void SendSysCommand(char text[])  
{ 
  delay(50);
  Serial.println(text);
  delay(100);
}

///Send a message to the BMSAIT App
void SendMessage(char message[], byte option)
{
  Serial.print(VAR_BEGIN);
  if (option==1)      //send a text message
  {
    Serial.print('t');
  }
  else if (option==2) //send a data request
  {
    Serial.print('d');
  }
  else if (option==3) //send a command
  {
    Serial.print('k');
  }
  else if (option==4) //sends analog axis data value
  {
    Serial.print('a');
  }
  else
  {
    //do nothing
  }
  Serial.print(message) ;
  Serial.println(VAR_ENDE);
}

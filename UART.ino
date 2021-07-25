
/***********************************************************************
Frame format

$|X|Y|Z|;

$ - 1 byte SOF -'$'
; - 1 byte EOF - ';'
X - 1 byte Command Type - 'C': SD Card Interface, 'D': WiFi Interface 
YZ - 0 - 16 bytes - X specific sub-Commands and data 

When X = C

Y - 1: Open SD Card and print open status on Console
Z - redundant or 0

Y - 2: Read the number of entries on the SD card and print the status on the debug console
Z - redundant or 0

Y - 3: Read the saved data on the SD card and print them on the debug console
Z - The Entry number (0-10000 or 5 days equivalent). 
  The s/w will pull the data of the mentioned entry number and display on the console 

Y - 4: Make new entry in the SD card and print it on the console 
Z - The time, switch, pir, lux, pwm data. This data will be stored on SD card 

Y - 5: Delete the entry in the SD card and print it on the console 
Z - The Entry number (0-10000 or 5 days equivalent) which is to be deleted.

Y - 6: Close SD Card and print open status on Console
Z - redundant or 0
*********************************Variable Decleration***************************************/
#include <Wire.h> //BH1750 I2C Mode 
#include <math.h>      
#include "DEBUG.h"
#include "SDCard.h"


void CMD_INFO(void);
void IRAM_ATTR onTimer(void);
void IRAM_ATTR IsSdCardMounted_Check(void);
void IRAM_ATTR ErrorIndication(void) ;


int BH1750address = 0x5C; //BH1750 Address
int Switch_Condition = 0; 
byte buff[2];


#define timeSeconds 10
#define I2C_SDA 16           //I2C SDA Pin  
#define I2C_SCL 17           //I2C SCL Pin
#define SWITCH1 19           // SW1 Input pin
#define SWITCH2 18           // SW2 Input pin  
#define SWITCH3 27           // SW3 Input pin

// Set GPIOs for LED and PIR Motion Sensor
const int led = 26;
const int motionSensor = 25;

// Timer: Auxiliary variables
unsigned long now = millis();
unsigned long lastTrigger = 0;
boolean startTimer = false;


unsigned char UbBuf[100];

unsigned int UhLen;
volatile int interruptCounter;
int totalInterruptCounter;
int ubCardWriteCntr;

bool ubCardCheckTimerExpired; 
bool ubCardWriteTimerExpired;
bool ubSdCardMountFail_Flag;
bool ubSdCardWriteFail_Flag;
bool ubSdCardMountFailCntr_Flag;
bool ubStartWriteErrorCntr_Flag;
bool ubSdCardReadFail_Flag;
bool ubStartReadErrorCntr_Flag;
bool ubSDNotAvailable_Flag;
bool ubSDNotAvailableCntr_Flag;

File file;
hw_timer_t * timer = NULL, *timer_1 = NULL;
portMUX_TYPE timerMux = portMUX_INITIALIZER_UNLOCKED;
extern E_SDCARD_FSM_STATE geSdcardFsm;
extern stDataPacket data_packet;

stDEBUGRX gstDebugRx;
E_DEBUG_FSM_STATE geDebuggerFsm = eDEBUG_FSM_STATE_READ;
E_DEBUG_READ_FSM_STATE geReadConsole = eDEBUG_READ_FSM_STATE_START;


// Checks if motion was detected, sets LED HIGH and starts a timer
void IRAM_ATTR detectsMovement() {
  Serial.println("MOTION DETECTED!!!");
  digitalWrite(led, HIGH);
  startTimer = true;
  lastTrigger = millis();
}


/********************************Serial Communication Setup ***************************************/
void setup()
{ 
  delay(1000);
  pinMode(SWITCH1,INPUT);
  pinMode(SWITCH2,INPUT);
  pinMode(SWITCH3,INPUT);
  Wire.begin(I2C_SDA,I2C_SCL);
  Serial.begin(115200); // Starts the serial communication 
  SDCard_Init(); 
  //CMD_INFO(); 
  Timer_Init();


  // PIR Motion Sensor mode INPUT_PULLUP
  pinMode(motionSensor, INPUT_PULLUP);
  // Set motionSensor pin as interrupt, assign interrupt function and set RISING mode
  attachInterrupt(digitalPinToInterrupt(motionSensor), detectsMovement, CHANGE);

  // Set LED to LOW
  pinMode(led, OUTPUT);
  digitalWrite(led, LOW);


  //geSdcardFsm = eSDCARD_FSM_STATE_READ;
  geSdcardFsm = eSDCARD_FSM_STATE_WRITE;
 // geSdcardFsm = eSDCARD_FSM_STATE_START;
  SDCard_FSM_Run();
 
}

void loop()
{  
  //READ_UDATA();  
  //DEBUG_FSM();
  //SDCard_Fsm();
   //Serial.println("/*****SD Card Interface Demo*****/\n");

   
    if(interruptCounter > 0)
    {
      portENTER_CRITICAL(&timerMux);
      interruptCounter--;
      portEXIT_CRITICAL(&timerMux);
     
      totalInterruptCounter++;
      Serial.print("An interrupt as occurred. Total number: ");
      Serial.println(totalInterruptCounter);
    }
}

/********************************User defined Function_1***************************************/
void Timer_Init(void)
{
  //-----TIMER 0-----------//
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 50000000, true);  // 50000000 = 50 sec; make it 5 mins after testing done
  timerAlarmEnable(timer);

  //-----TIMER 1-----------//
  timer_1 = timerBegin(1, 80, true);
  timerAttachInterrupt(timer_1, &IsSdCardMounted_Check, true);
  timerAlarmWrite(timer_1, 1000000, true);  // 3000000 = 3 sec;
  timerAlarmEnable(timer_1); 
   
  //-----TIMER 2-----------//
  timer_1 = timerBegin(2, 80, true);
  timerAttachInterrupt(timer_1, &ErrorIndication, true);
  timerAlarmWrite(timer_1, 1000000, true);  // 3000000 = 3 sec;
  timerAlarmEnable(timer_1);  
  
}

void IRAM_ATTR onTimer(void) 
{
  portENTER_CRITICAL_ISR(&timerMux); 
  //TO DO: Use this for every 5 mins and write data in sdcard file
  //ubCardWriteCntr++;
  //interruptCounter++;
  ubCardWriteTimerExpired = TRUE;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void IRAM_ATTR IsSdCardMounted_Check(void) 
{
  //TO DO: Add sd card is available or not check here for every 1 sec 
  portENTER_CRITICAL_ISR(&timerMux);
  ubCardCheckTimerExpired = TRUE;
  portEXIT_CRITICAL_ISR(&timerMux);
}

void IRAM_ATTR ErrorIndication(void) 
{
  //TO DO: Add sd card is available or not check here for every 1 sec 
  portENTER_CRITICAL_ISR(&timerMux);
  //SDCard_FSM_Run();
  if(TRUE == ubSdCardMountFailCntr_Flag)
  {
    Serial.println("+++++SD CARD MOUNT FAILED++++");
  }

  if(TRUE == ubStartWriteErrorCntr_Flag)
  {
    Serial.println("+++++***SD CARD WRITE FAILED***++++");
  }

  if(TRUE == ubStartReadErrorCntr_Flag)
  {
    Serial.println("+++++***SD CARD READ FAILED***++++");
  }

  if(TRUE == ubSDNotAvailableCntr_Flag)
  {
    Serial.println("+++++***SD CARD MOUNT FAILED***++++");
  }
  
  portEXIT_CRITICAL_ISR(&timerMux);
}


/********************************User defined Function_2***************************************/
void DEBUG_FSM()
{
  unsigned char UbRet;
  switch(geDebuggerFsm)
  {
    case eDEBUG_FSM_STATE_READ:    
      //Read UART Frame and copy to Local Data Buffer  
      if(eDEBUG_FRAME_STATE_RECEIVED == gstDebugRx.eFrameState) 
      {                
        geDebuggerFsm = eDEBUG_FSM_STATE_CMD_DECODE;        
      }
      break;
      /*
    case eDEBUG_FSM_STATE_FRAMECHECK:      
      if(gstDebugRx.UbData[0] == '$')
      {
        geDebuggerFsm = eDEBUG_FSM_STATE_CMD_DECODE;
        #ifdef D_EN_DEBUG_CONSOLE
        Serial.println("eDEBUG_FSM_STATE_FRAMECHECK");
        #endif
      }
      else
      {
        //Wrong Frame 
        //Clear Buffer
        #ifdef D_EN_DEBUG_CONSOLE
        Serial.println(gUbData);
        //Serial.println("Wrong Frame");
        #endif
        geDebuggerFsm = eDEBUG_FSM_STATE_READ;
      }
      break;    
      */
    case eDEBUG_FSM_STATE_CMD_DECODE:   
      switch(gstDebugRx.UbData[1])
      {
        case 'C': //Commands to Debug/Demo the SD Card Interface                 
          switch(gstDebugRx.UbData[2])
          {           
            case '1': 
              file = SD.open("/SAMSUN.txt", FILE_WRITE);
              if(file)
              {
               // while(file.available())
                {
                  //Serial.println("SAMSUN.txt Opened");
                  if(file.seek(1))
                  {
                    //UbBuf[0] = file.read(); 
                    //Serial.println(UbBuf[0]);
                   // while (file.available()) 
                    { //execute while file is available
                      char letter = (char)file.read(); //read next character from file
                      Serial.print(letter); //display character
                      delay(300);
                    }
                  }
                  else
                  {
                    Serial.println("file.seek didn't work");
                  }
                }
              }
              else
              {
                Serial.println("SAMSUN.txt Not Opened");
              }
              file.close();
              break;
              
            case '2': //Read the number of entries on the SD card and print the status on the debug console

              break;
                
            case '3': //Read the saved data on the SD card and print them on the debug console

              break;            
            case '4': //Make new entry in the SD card and print it on the console 

              break;
            case '5': //Delete the specific entry on the SD card 

              break;
            case '6': //Close the SD card 
  
              break;
              
            default:
            Serial.println("Invalid Command");
            break;
          }
          geDebuggerFsm = eDEBUG_FSM_STATE_IDLE;
          break;
          
        case 'D': //Commands to Debug/Demo the Wifi In
          Serial.println("Command Not Implemented");
          geDebuggerFsm = eDEBUG_FSM_STATE_IDLE;
          break;
        
        default:
          Serial.println("Command Not Implemented");
          geDebuggerFsm = eDEBUG_FSM_STATE_IDLE;
          break;          
      }
      break;    
      
    case eDEBUG_FSM_STATE_IDLE:
      gstDebugRx.eFrameState = eDEBUG_FRAME_STATE_EMPTY;
      geDebuggerFsm = eDEBUG_FSM_STATE_READ;
      break;  
    
    default:
      break;    
  } 
}

//---------------------------------------------SENSOR CODE------------------------------------//
void PIR_Measure(void)
{
  // Current time
  now = millis();
  // Turn off the LED after the number of seconds defined in the timeSeconds variable
  if(startTimer && (now - lastTrigger > (timeSeconds*1000))) {
    Serial.println("Motion stopped...");
    digitalWrite(led, LOW);
    startTimer = false;
    Switch_Condition = 1;
  }
}

uint16_t Lux_Measure(void)
{
  int i;
  uint16_t val=0;
  BH1750_Init(BH1750address);
  delay(200);

  if(2==BH1750_Read(BH1750address))
  {
    
      val=((buff[0]<<8)|buff[1])/1.2;
      val=val+15;
      Serial.println("VAL="); 
    Serial.print(val,DEC);     
    Serial.println("[lx]"); 
  }
  delay(150);
  Switch_Condition = 0;
  return val;
}


int BH1750_Read(int address) //
{
  int i=0;

  Serial.println(F("Read function: "));   // added
  
  Wire.requestFrom(address, 2);
  while(Wire.available()) //
  {
    buff[i] = Wire.read();  // receive one byte
 
    Serial.print(buff[i], DEC);   // added
    Serial.print(F(", "));   // added

    i++;
  }
  Serial.println();

  return i;
}
void BH1750_Init(int address) 
{
  Wire.beginTransmission(address);
  Wire.write(0x10);//1lx reolution 120ms
  Wire.endTransmission();
}

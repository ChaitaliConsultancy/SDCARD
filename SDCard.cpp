
#include "DEBUG.h"
#include "SDCard.h"
#include <string.h>

/*********************Global Variables Begin*************************/
E_SDCARD_FSM_STATE geSdcardFsm = eSDCARD_FSM_STATE_START; 
stSDCARD gstSdCard;
stDataPacket data_packet;   //used for creating data packet

const char aDefaultFileName[] = "/180721.txt";
char aRDBUFF[25];
char aRDBUFF_t1[50];
unsigned char aDEVICE_ID[] = "ALC1234";

unsigned int uhwRedDutyCycle;
unsigned int uhwGreenDutyCycle;
unsigned int uhwBlueDutyCycle;
unsigned char UbFileRequest=0;
/*********************Global Variables End*************************/
          
bool SDCard_Init(void)
{
    uint8_t cardType;
    uint64_t cardSize;
    
    if(!SD.begin())
    {
        Serial.println("Card Mount Failed");     
        return false;   
    }
    else
    {
        Serial.println("Card Mount Successfully");
        cardType = SD.cardType();
    
        if(cardType == CARD_NONE)
        {
            Serial.println("No SD card attached");        
        }
        Serial.print("SD Card Type: ");
        if(cardType == CARD_MMC){
            Serial.println("MMC");
        } else if(cardType == CARD_SD){
            Serial.println("SDSC");
        } else if(cardType == CARD_SDHC){
            Serial.println("SDHC");
        } else {
            Serial.println("UNKNOWN");
        }
        
        cardSize = SD.cardSize() / (1024 * 1024);
        Serial.printf("SD Card Size: %lluMB\n", cardSize);
        
        return true;
    }
   

}


bool WriteDataPacket_To_SDCard(const char *fileName,stDataPacket sensor_data)
{
    File fileWrId;
    //int  date_dd, date_mm, date_yy, time_hh, time_mm, time_ss,lux_data,OCC1, OCC2, OCC3, RPWM, GPWM, BPWM, ERROR_st;
    fileWrId = SD.open(fileName, FILE_WRITE);
    if(1 == fileWrId)
    {
        Serial.println("+++++File Opened for writing Successfully+++++"); 
        /*
         *#
          Device ID: ALCXXXX
          File Name: /14164200.txt
          Date: Wednesday, July 14 2021 16:42:26
          Time,Ambient,OCC1,OCC2,OCC3,RPWM,GPWM,BPWM,ERROR
          15,07,21,10,32,36,0080,1,0,1,50,50,40,0000
        */
          fileWrId.print("# \n");
          fileWrId.print("Device ID: ");   
          fileWrId.write(aDEVICE_ID[0]);
          fileWrId.write(aDEVICE_ID[1]);
          fileWrId.write(aDEVICE_ID[2]);
          fileWrId.write(aDEVICE_ID[3]);
          fileWrId.write(aDEVICE_ID[4]);
          fileWrId.write(aDEVICE_ID[5]);
          fileWrId.write(aDEVICE_ID[6]);
          //fileWrId.print("\"\");
          fileWrId.print("\n");
          fileWrId.print("File Name: ");
          fileWrId.println(fileName);
          fileWrId.print("Date: Wednesday, July 14 2021 16:42:26 \n"); //make dynamic
          fileWrId.print("dd,mm,yy,hh,mm,ss,AMBEINT,OCC1,OCC2,OCC3,RPWM,GPWM,BPWM,ERROR \n");
          fileWrId.print(sensor_data.date_n.dd);
          fileWrId.print(",");
          fileWrId.print(sensor_data.date_n.mm);
          fileWrId.print(",");
          fileWrId.print(sensor_data.date_n.yy);
          fileWrId.print(",");
          fileWrId.print(sensor_data.time_n.hh);
          fileWrId.print(",");
          fileWrId.print(sensor_data.time_n.mm);
          fileWrId.print(",");
          fileWrId.print(sensor_data.time_n.ss);
          fileWrId.print(",");
           
           if(sensor_data.lux_val < 99)
           {
              fileWrId.print("00");
              fileWrId.print(sensor_data.lux_val);
           }
           else if(sensor_data.lux_val < 990)
           {
              fileWrId.print("0");
              fileWrId.print(sensor_data.lux_val);
           }
           else
           {
              fileWrId.print(sensor_data.lux_val);
           }
           
          fileWrId.print(",");
          fileWrId.print(sensor_data.OCC1);
          fileWrId.print(",");
          fileWrId.print(sensor_data.OCC2);
          fileWrId.print(",");
          fileWrId.print(sensor_data.OCC3);
           fileWrId.print(",");
          fileWrId.print(sensor_data.RPWM);
           fileWrId.print(",");
          fileWrId.print(sensor_data.GPWM);
           fileWrId.print(",");
          fileWrId.print(sensor_data.BPWM);
           fileWrId.print(",");
          fileWrId.print(sensor_data.ERROR_n);
          fileWrId.print("\n");

          Serial.println("Writing Done...");
          fileWrId.close();
          return true;
    }
    else
    {
      Serial.println("Not Opened");
      return false;
    }

}


bool ReadDataPacket_From_SDCard(const char *nameOfFile, char *aRdBuf)
{
    File fileRdId;
    uint16_t stringIndex = 0, printIndex;
    uint8_t nextByteToRead;
    
    fileRdId = SD.open(nameOfFile);
    if(fileRdId) 
    {
      Serial.print("File Opened successfully for reading \n");
      Serial.print("Reading DATA from:");
      Serial.println(nameOfFile);
      
      // read from the file until there's nothing else in it:
      while(fileRdId.available())
      {  
        //get the next character
        nextByteToRead = fileRdId.read();
        delay(10);
        Serial.write(nextByteToRead);   //comment it after testing done
        
        aRdBuf[stringIndex] = nextByteToRead; 
        stringIndex++;            
      }
      Serial.println("Reading Completed");
   
      // close the file:
      fileRdId.close();

      return true;
    } 
    else 
    {
        // if the file didn't open, print an error:
        Serial.println("error opening ");
        Serial.println(nameOfFile);
        return false;
    }
}


void SDCard_FSM_Run(void)
{
    bool ret_t;
    bool sd_status;
    //uint8_t aRDBUFF_t[50] = {0};
    String buffer_t;
    
  switch(geSdcardFsm)
  {   
        case eSDCARD_FSM_STATE_START:
        {
          File fileId_n;
          char *pDEVICEID_MATCH = NULL;
          unsigned char index;
          //Initialization
          
          Serial.println("+++++++In eSDCARD_FSM_STATE_START++++++");
          
          sd_status = SDCard_Init(); 
          if(sd_status == true) 
          {
              //mount success
              ubSdCardMountFail_Flag = FALSE;
            
              Serial.println("+++++++mount success++++++");
                //call read func to get unique id
              ReadDataPacket_From_SDCard(aDefaultFileName,aRDBUFF); // this func will give data from file

              pDEVICEID_MATCH = aRDBUFF+14;           
              if(strstr(aRDBUFF, "#") != NULL)
              {
                if(strstr(aRDBUFF, "Device ID:")!= NULL)
                {
                    //Need to add manufacturer Device id to compare
                    if((pDEVICEID_MATCH[0] == 'A') && (pDEVICEID_MATCH[1] == 'L') && (pDEVICEID_MATCH[2] == 'C') && (pDEVICEID_MATCH[3] == '1') 
                    && (pDEVICEID_MATCH[4] == '2') && (pDEVICEID_MATCH[5] == '3') && (pDEVICEID_MATCH[6] == '4'))
                    {
                        Serial.println("*******Device ID Matchiiing*******");
                        //go to idle state
                        geSdcardFsm = eSDCARD_FSM_STATE_WAIT;
                        ubSdCardMountFail_Flag = FALSE;
                        timerAlarmEnable(timer); //5 mins timer starts here
                    }
                    else
                    {
                        //device id doesnt match, erase everything
                        //for(index = 0; index<5; index++)
                        {
                          SD.remove(aDefaultFileName);
                        }
                        //go to write new device id
                        fileId_n = SD.open(aDefaultFileName, FILE_WRITE);
                        if(1 == fileId_n)
                        {
                            Serial.println("+++++File Opened Successfully+++++"); 
                            geSdcardFsm = eSDCARD_FSM_STATE_WRITE;
                            ubSdCardWriteFail_Flag = FALSE;
                            
                            fileId_n.close();
                        }
                        else
                        {
                            //write fail 
                            Serial.println("+++++File not Opened Successfully+++++"); 
                            geSdcardFsm = eSDCARD_FSM_STATE_ERROR;
                            ubSdCardWriteFail_Flag = TRUE;
                        }
                      //------------------------------------------------//
                  }  
                    
                }//end of device id if
                
              }//end of "#" if
           
          }
          else
          {
              geSdcardFsm = eSDCARD_FSM_STATE_ERROR;
              ubSdCardMountFail_Flag = TRUE;
          }
 
        
          //geSdcardFsm = eSDCARD_FSM_STATE_OPEN;
        }
        break;
//--------------------------------------------
    case eSDCARD_FSM_STATE_OPEN:
      if(UbFileRequest == 1)
      {
        //File will get open and state change for reading otherwise error state
        SDCardFile_Read_Request(aDefaultFileName);
      }
      else if (UbFileRequest == 2)
      {
        //file will get open and state change will happen for writing otherwise error state
        SDCardFile_Append_Request(aDefaultFileName);
      }
      break;

        case eSDCARD_FSM_STATE_READ:
        {
            ret_t = ReadDataPacket_From_SDCard(aDefaultFileName,aRDBUFF_t1);
            if(ret_t == 1)
            {
              //read success
              //TO DO: sending data on wifi
              ubSdCardReadFail_Flag = FALSE;
            }
            else
            {
               //read fail
               geSdcardFsm = eSDCARD_FSM_STATE_ERROR;
               ubSdCardReadFail_Flag = TRUE;
            }
            //geSdcardFsm = eSDCARD_FSM_STATE_CLOSE;
        }
        break;

    case eSDCARD_FSM_STATE_WRITE:
    { 
        bool ret;
        uint16_t Lux_Measure_Result;
        //read sensor data
        // make packet
        ubCardWriteTimerExpired = false;
        Serial.println("In Write state...");
        
//        data_packet.data_n.dd = date_from_wifi;
//        data_packet.data_n.mm = month_from_wifi;
//        data_packet.data_n.yy = year_from_wifi;
//        data_packet.time_n.hh = hour_from_wifi;
//        data_packet.time_n.mm = min_from_wifi;
//        data_packet.time_n.ss = sec_from_wifi;
//        data_packet.OCC1 = pir1;
//        data_packet.OCC2 = pir2;
//        data_packet.OCC3 = pir3;
//        data_packet.RPWM = rpwm;
//        data_packet.GPWM = gpwm;
//        data_packet.BPWM = bpwm;
//        data_packet.ERROR_n = error;

        //Prepare data packet for writing
        Lux_Measure_Result = Lux_Measure();

        data_packet.date_n.dd = 24;
        data_packet.date_n.mm = 07;
        data_packet.date_n.yy = 21;
        data_packet.time_n.hh = 11;
        data_packet.time_n.mm = 49;
        data_packet.time_n.ss = 06;
        data_packet.lux_val = Lux_Measure_Result;
        data_packet.OCC1 = 0;
        data_packet.OCC2 = 1;
        data_packet.OCC3 = 0;
        data_packet.RPWM = uhwRedDutyCycle;
        data_packet.GPWM = uhwGreenDutyCycle;
        data_packet.BPWM = uhwBlueDutyCycle;
        data_packet.ERROR_n = 1;

        
        ret = WriteDataPacket_To_SDCard(aDefaultFileName,data_packet);
        if(ret == 1)
        {
            //write success
            Serial.println("write 1 attempt success...");
            ubSdCardWriteFail_Flag = FALSE;
            geSdcardFsm = eSDCARD_FSM_STATE_WAIT;
        }
        else
        {
          //one more write attempt
          Serial.println("One more Write Attempt...");
          ret = WriteDataPacket_To_SDCard(aDefaultFileName,data_packet);
          if(ret == 1)
          {
              //write success
              Serial.println("write 2 attempt success...");
              ubSdCardWriteFail_Flag = FALSE;
              geSdcardFsm = eSDCARD_FSM_STATE_WAIT;
              //break;
          }
          else
          {
             //write fail 
             Serial.println("write attempt Failed...");
             geSdcardFsm = eSDCARD_FSM_STATE_ERROR;
             ubSdCardWriteFail_Flag = TRUE;
          }
        }
        //if write fails, attempt one more time
        // if second write attempt fails, go to error state 
        // in error state every 1 second, print on console or to wifi that SD card write error

        //on write completion go to idle state
    }
      break;      

    case eSDCARD_FSM_STATE_WAIT: //wait
        if(TRUE == ubCardCheckTimerExpired)
        {
          ubCardCheckTimerExpired = FALSE;
          uint8_t cardType = SD.cardType();
          if(cardType != CARD_NONE)
          {
              ubSDNotAvailable_Flag = FALSE; 
              Serial.print(cardType);
              Serial.println(" SD card is attached");        
          }
          else
          {
              ubSDNotAvailable_Flag = TRUE;
              geSdcardFsm = eSDCARD_FSM_STATE_ERROR;
          }
          //if card isn't mounted or removed, go to error state
          //in error state every 1 second write to console or to wifi that SD card not present error
        }
    
        if(TRUE == ubCardWriteTimerExpired)
        {
            geSdcardFsm = eSDCARD_FSM_STATE_WRITE; 
        }
    break;
    
    case eSDCARD_FSM_STATE_CLOSE:
      break;     

    case eSDCARD_FSM_STATE_ERROR:
      {
        //Serial.println("In eSDCARD_FSM_STATE_ERROR");
        if(ubSdCardMountFail_Flag == TRUE)
        {
            ubSdCardMountFailCntr_Flag = TRUE;
        }

        if(ubSdCardWriteFail_Flag == TRUE)
        {
            ubStartWriteErrorCntr_Flag = TRUE;
        }

        if(ubSdCardReadFail_Flag == TRUE)
        {
            ubStartReadErrorCntr_Flag = TRUE;
        }

        if(ubSDNotAvailable_Flag == TRUE)
        {
          ubSDNotAvailableCntr_Flag = TRUE;
        }
      }
      break;


//--------------------------------------------    
    default:
       break;
  }
}

void SDCardFile_Read_Request(const char *Filename)
{
    gstSdCard.Files[0] = SD.open(Filename, FILE_READ);
    if(gstSdCard.Files[0] > 0)
    {
      Serial.print(Filename);
      Serial.println(" opened in Read Mode from SDCardFile_Read_Request");        
    }
    else
    {
      Serial.print(Filename);
      Serial.println(" opening error from SDCardFile_Read_Request");
      geSdcardFsm = eSDCARD_FSM_STATE_ERROR;
    }
    geSdcardFsm = eSDCARD_FSM_STATE_READ;
}

void SDCardFile_Append_Request(const char *Filename)
{
    gstSdCard.Files[0] = SD.open(Filename, FILE_APPEND);
    if(gstSdCard.Files[0] > 0)
    {
      Serial.print(Filename);
      Serial.println(" opened in Write Mode from SDCardFile_Append_Request"); 
      geSdcardFsm = eSDCARD_FSM_STATE_WRITE;       
    }
    else
    {
       Serial.print(Filename);
      Serial.println(" opening error from SDCardFile_Append_Request");
      //geSdcardFsm = eSDCARD_FSM_STATE_ERROR;
    }          
    
}

bool SDCard_IsFileExist(const char* pstrFileName)
{
  Serial.println(pstrFileName);
  if (SD.exists(pstrFileName)) 
  {    
    Serial.println("exists.");
    return TRUE;
  } 
  else 
  {    
    Serial.println("doesn't exist.");
    return FALSE;
  }
}


bool SDCard_OpenFile(unsigned char UbMode)
{
  bool ret;
  switch(UbMode)
  {
    case 1:
      gstSdCard.Files[0] = SD.open("/SampleFile.txt", FILE_READ);
      if(gstSdCard.Files[0] > 0)
      {
        ret = TRUE;              
      }
      else
      {
        ret = FALSE;            
      }    
      break;

    case 2:
      gstSdCard.Files[0] = SD.open("/File.txt", FILE_APPEND);
      if(gstSdCard.Files[0] > 0)
      {
        ret = TRUE;
        Serial.println("File.txt opened in Write Mode");        
      }
      else
      {
        ret = FALSE;
        Serial.println("File.txt opening error");
        geSdcardFsm = eSDCARD_FSM_STATE_ERROR;
      }    
      break;  

    default:
      break;  
  }
  return ret;
}


void SDCard_CloseFile(void)
{
  gstSdCard.Files[0].close();
}
/*
void WriteFile()
{
  DAY1 = SD.open("DAY1.txt", FILE_WRITE);
  if(DAY1){
    Serial.print("Writing to DAY1.txt...");
    DAY1.println("Time Instance in Sec, LUX, PIR,SW1,SW2,SW3,SW4,PWM Duty \r\n");
    for (IHwEntryCount = 0; IHwEntryCount <= 30; IHwEntryCount++){
    WriteToSD(IWLUX[IHwEntryCount],IWPIR[IHwEntryCount],IHwSW1[IHwEntryCount],IHwSW2[IHwEntryCount],IHwSW3[IHwEntryCount],IHwSW4[IHwEntryCount],IHwPWM[IHwEntryCount]);
    appendFile(SD, "/DAY1.txt", dataString.c_str());
    }
  }
  DAY1.close();
  
  DAY2 = SD.open("DAY1.txt", FILE_WRITE);
  if(DAY2){
    Serial.print("Writing to DAY1.txt...");
    DAY2.println("Time Instance in Sec, LUX, PIR,SW1,SW2,SW3,SW4,PWM Duty \r\n");
    for (IHwEntryCount = 0; IHwEntryCount <= 30; IHwEntryCount++){
    WriteToSD(IWLUX[IHwEntryCount],IWPIR[IHwEntryCount],IHwSW1[IHwEntryCount],IHwSW2[IHwEntryCount],IHwSW3[IHwEntryCount],IHwSW4[IHwEntryCount],IHwPWM[IHwEntryCount]);
    appendFile(SD, "/DAY2.txt", dataString.c_str());
    }
  }
  DAY2.close();
}

void WriteToSD(long int IWvar1,long int IWvar2,int IHwvar3,int IHwvar4,int IHwvar5,int IHwvar6,int IHwvar7){
   // String dataString = "";
    dataString += String(IWTimeInstance);
    dataString += ",";
    dataString += String(IWvar1);
    dataString += ",";
    dataString += String(IWvar2);
    dataString += ",";
    dataString += String(IHwvar3);
    dataString += ",";
    dataString += String(IHwvar4);
    dataString += ",";
    dataString += String(IHwvar5);
    dataString += ",";
    dataString += String(IHwvar6);
    dataString += ",";
    dataString += String(IHwvar7);
    dataString += "\r\n";
    Serial.println(dataString);
//    appendFile(SD, "/datalog_ESP32.txt", dataString.c_str());
    IWTimeInstance += 3;
}
*/


#ifndef SDCARD_H
#define SDCARD_H

#include <SPI.h>
#include <SD.h>
#include <FS.h>

#define D_NO_OF_FILES 5

typedef enum{
  eSDCARD_FSM_STATE_START,
  eSDCARD_FSM_STATE_WAIT,
  eSDCARD_FSM_STATE_OPEN,  
  eSDCARD_FSM_STATE_READ,
  eSDCARD_FSM_STATE_WRITE,
  eSDCARD_FSM_STATE_CLOSE,
  eSDCARD_FSM_STATE_ERROR
}E_SDCARD_FSM_STATE;

typedef struct{
  File Files[D_NO_OF_FILES];
  unsigned int UwEntryCounter[D_NO_OF_FILES];
  unsigned int UwEntryIndex[D_NO_OF_FILES];
}stSDCARD;

typedef struct
{
  unsigned char dd;
  unsigned char mm;
  unsigned char yy;
}stDate;

typedef struct
{
  unsigned char hh;
  unsigned char mm;
  unsigned char ss;
}stTime;

typedef struct
{
  stDate date_n;
  stTime time_n;
  unsigned int lux_val;
  unsigned char OCC1;
  unsigned char OCC2;
  unsigned char OCC3;
  unsigned int RPWM;
  unsigned int GPWM;
  unsigned int BPWM;
  unsigned int ERROR_n;
}stDataPacket;

/*----------VARAIBLE DECLARATON----------*/
extern unsigned char UbFileRequest;
extern const char aDefaultFileName[];
extern bool ubCardCheckTimerExpired; 
extern bool ubCardWriteTimerExpired;
extern bool ubSdCardMountFail_Flag;
extern bool ubSdCardWriteFail_Flag;
extern bool ubSdCardMountFailCntr_Flag;
extern bool ubStartWriteErrorCntr_Flag;
extern bool ubStartReadErrorCntr_Flag;
extern bool ubSdCardReadFail_Flag;
extern bool ubSDNotAvailable_Flag;
extern bool ubSDNotAvailableCntr_Flag;
extern hw_timer_t * timer;

/*------FUNCTION DECLARATIONS------------*/
extern bool SDCard_Init(void);
extern void SDCard_Fsm(void);
extern bool SDCard_IsFileExist(const char* pstrFileName);
extern bool SDCard_OpenFile(unsigned char UbMode);
extern void SDCard_CloseFile(void);
extern bool ReadDataPacket_From_SDCard(const char *nameOfFile, char *aRdBuf);
extern bool WriteDataPacket_To_SDCard(const char *fileName,stDataPacket sensor_data);
extern void SDCard_FSM_Run(void);
extern void SDCardFile_Read_Request(const char *Filename);
extern void SDCardFile_Append_Request(const char *Filename);

//--------------------------------------------SENSOR DECLARATION----------------------------//
extern void PIR_Measure(void);
extern uint16_t Lux_Measure(void);
extern int BH1750_Read(int address);
extern void BH1750_Init(int address);

#endif //#ifdef SDCARD_H

//#define D_EN_DEBUG_CONSOLE
#define D_EN_DEBUG_CONOLSE_SD_CARD
#define DATA_SIZE 20

typedef enum{
    eDEBUG_READ_FSM_STATE_START,  
    eDEBUG_READ_FSM_STATE_SOF,
    eDEBUG_READ_FSM_STATE_READ,
    eDEBUG_READ_FSM_STATE_IDLE
}E_DEBUG_READ_FSM_STATE;

typedef enum{
    FALSE = 0, 
    TRUE = !FALSE
};

typedef enum{
    eDEBUG_FSM_STATE_READ,  
    eDEBUG_FSM_STATE_FRAMECHECK,
    eDEBUG_FSM_STATE_CMD_DECODE,
    eDEBUG_FSM_STATE_IDLE
}E_DEBUG_FSM_STATE;

typedef enum{
  eDEBUG_FRAME_STATE_EMPTY = 0,
  eDEBUG_FRAME_STATE_RECEIVED
}E_DEBUG_FRAME_STATE;

typedef struct{
  unsigned char UbData[DATA_SIZE];
  unsigned char UbIndex;
  unsigned char UbLen;
  E_DEBUG_FRAME_STATE eFrameState;
}stDEBUGRX;

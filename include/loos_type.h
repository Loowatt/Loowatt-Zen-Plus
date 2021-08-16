#ifndef LOOS_TYPE_H_
#define LOOS_TYPE_H_

#include <Arduino.h>

#define  SLAVE_ADDRESS      0x48

#define MAXBYTEEXPECTED 4

// I2C Register Addresses
#define UID_ADDR 0x00               //R 2
#define STATUS_ADDR 0x01            //R-W 1

#define MOTORCTRL_ADDR 0x02         //R-W 1

#define JAM_THRESH_CPS_ADDR 0x03    //R-W 2 //below this count, the jam state would be raised and system will be put into service mode
#define COUNT_MM_ADDR 0x04          //R-W 2 
#define MAX_FLUSH_COUNT_ADDR 0x05   //R-W 2
#define MAX_FILM_COUNT_ADDR 0x06    //R-W 4

#define FILM_LEFT_ADDR 0x07         //R-W 4 // current film count
#define MOTOR_STATE_ADDR 0x08       //R 2
#define TEMP_ADDR 0x09              //R 2

#define LT_FLUSH_ADDR 0x10          //R 4
#define LT_FILM_USED_ADDR 0x11      //R 4
#define LT_BLOCK_ADDR  0x12         //R 2
#define LT_SER_ADDR 0x13            //R 2


/*

/////BLE DATA/////

#define MAX_FILM_COUNT_ADDR       0x14 //W 4
#define ACTUAL_FILM_LENGTH_ADDR   0x15 //W 4
#define USABLE_FILM_LENGTH_ADDR   0x16 //W 4



*/

typedef enum
{
    ME_INITIALISING,
    ME_RUN,     // State, film left - Only mode button active/ flush active
    ME_SERVICE, //State of System - disable flush/ foward and backward active
    ME_REFILM,  // internal navigation and confirmation of film change
    ME_JAM, //A menu which is active, only if a jam has been detected
    ME_SUDATA,  // A hidden menu, displaying SU data stored on EEPROM
    ME_REFILL_CONFIRMATION,
    ME_JAM_CONFIRMATION, 
    ME_WIFI_SETUP,
    ME_WIFI_YES,
    ME_WIFI_NO,
} T_MenuState;

typedef enum 
{
  T_UID = UID_ADDR,
  T_STATUS = STATUS_ADDR,
  T_MOTORCTRL = MOTORCTRL_ADDR,
  T_JAM_THRESH = JAM_THRESH_CPS_ADDR,
  T_COUNT_MM = COUNT_MM_ADDR,
  T_MAX_FLUSH_COUNT = MAX_FLUSH_COUNT_ADDR,
  T_FILM_LEFT = FILM_LEFT_ADDR,
  T_MOTOR_CURRENT = MOTOR_STATE_ADDR,
  T_TEMP = TEMP_ADDR,
  T_LT_FLUSH = LT_FLUSH_ADDR,
  T_LT_FILM = LT_FILM_USED_ADDR,
  T_LT_BLOCK = LT_BLOCK_ADDR,
  T_LT_SER = LT_SER_ADDR,

} T_ADDR;

union T_MACData
{
  struct
  {
    byte byte_0: 8;
    byte byte_1: 8;
    byte byte_2: 8;
    byte byte_3: 8;
    byte byte_4: 8;
    byte byte_5: 8;
    byte byte_6: 8;
    byte byte_7: 8;
  } s_eightbytes;
  unsigned long long u_eightbytes;
};

union T_TwoBytesData
{
  struct
  {
    byte byte_0: 8;
    byte byte_1: 8;
  } s_twobytes;
  unsigned short u_twobytes;
};

union T_FourBytesData
{
  struct
  {
    byte byte_0: 8;
    byte byte_1: 8;
    byte byte_2: 8;
    byte byte_3: 8;
  } s_fourbytes;
  unsigned long u_fourbytes;
};

union T_FourBytesData_Signed
{
  struct
  {
    byte byte_0: 8;
    byte byte_1: 8;
    byte byte_2: 8;
    byte byte_3: 8;
  } s_fourbytes;
  signed long u_fourbytes;
};

// internal tracking data

typedef enum 
{
  S_RUN, 
  S_SER, 
  S_STP, 
  S_UN
}e_StateType;
const char* e_StateTypeToString[] = {"RUN", "SERVICE", "STP", "UN"};

typedef enum
{
  INIT,
  NOTINIT
}e_IsInitType;
const char* e_IsInitTypeToString[] = {"INIT",  "NOTINIT"};


typedef enum
{
  ENGAGED, 
  VACANT
}e_EngagedFlagType;
const char* e_EngagedFlagTypeToString[] = {"ENGAGED", "VACANT"};

typedef enum
{
  NOJAM, 
  JAM
}e_JamFlagType;
const char* e_JamFlagTypeToString[] = {"NOJAM", "JAM"};

typedef enum
{
  NOTEMPTY, 
  EMPTY
}e_FilmStateType;
const char* e_FilmStateTypeToString[] = {"NOTEMPTY", "EMPTY"};


typedef enum
{
  ON, 
  OFF
}e_PeriphStateType;
const char* e_PeriphStateTypeToString[] = {"ON", "OFF"};

union T_StatusByte
{
  struct
  {
    byte state : 2;
    byte IsInit: 1;
    byte busy : 1;
    byte jam : 1;
    byte film : 1;
    byte fan : 1;
    byte light : 1;
  }s_status;
  byte u_status;
};



union T_MotorControlByte
{
  struct
  {
    byte m_state : 2;
    byte flush : 1;
  }s_motorcontrol;
  byte u_motorcontrol;
};


typedef enum 
{
  MSTOP, 
  MBACKWARD, 
  MFORWARD, 
  MBRAKE
}m_StateType;

typedef enum
{
  MNOTFLUSH, 
  MFLUSH
} e_FlushType;
const char* e_FlushTypeToString[] = {"NOTFLUSH", "FLUSH"};


enum e_TwoWireTransmissionType{SUCC, TL, NACK_ADDR, NACK_DATA, OTHER};
const char* e_TwoWireTransmissionTypeToString[] = {"SUCC", "TL", "NACK_ADDR", "NACK_DATA", "OTHER"};


#endif /* LOOS_TYPE_H_ */

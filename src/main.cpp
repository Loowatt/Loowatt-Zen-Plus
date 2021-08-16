#include <Arduino.h>
#include <ArduinoBLE.h>
#include <FlashStorage.h>
#include <U8g2lib.h>
#include <SPI.h>
#include <Wire.h>
#include <TaskScheduler.h>
#include <WiFiNINA.h>
#include <PubSubClient.h>
#include <avr/dtostrf.h>
#include "Button.h"
#include "loos_type.h"
#include "pins.h"
#include "i2c.h"
#include <string.h>
#include "Sodaq_wdt.h"

#define MQTT_MAX_PACKET_SIZE 1024

typedef struct
{
  boolean isValid;
  char ssid[20];
} WifiSSIDStore;

typedef struct 
{
  boolean isValid;
  char password[20];
} WifiPassStore;

typedef struct 
{
  boolean isValid;
  char token[500];
} TokenPassStore;


bool ShowWifi = false;
bool isWifiOn = false;
int WifiConfig;


FlashStorage(wifi_config_store, bool); 

FlashStorage(ssid_flash_store, WifiSSIDStore);
FlashStorage(pass_flash_store, WifiPassStore);
FlashStorage(token_flash_store, TokenPassStore);

int FlushDuration = 1500;
int NewFlushDuration;
FlashStorage(flush_duration_store, int);


bool isBleOn = false;

BLEDevice central;

BLEService BLEWifi("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service
BLEStringCharacteristic ssidCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", 12, 128);
BLEStringCharacteristic passwordCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1215", 12, 128);
BLEStringCharacteristic flushCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1216", 12, 128);  //flush duration  (1000) (1500) (2000)
BLEStringCharacteristic tokenCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1217", 12, 128); //film length 8m,16m


void blePeripheralConnectHandler(BLEDevice central)
{

  Serial.print("Connected event, central: ");
  Serial.println(central.address());


}

void blePeripheralDisconnectHandler(BLEDevice central)
{

  Serial.print("disconnected event, central: ");
  Serial.println(central.address());
}


void ssidCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic)
{
  WifiSSIDStore wifi_store; 
  Serial.print("Characteristic event, written: ");

  if(ssidCharacteristic.value())
  {
    Serial.println(ssidCharacteristic.value());

    ssidCharacteristic.value().toCharArray(wifi_store.ssid, ssidCharacteristic.valueSize());
    wifi_store.isValid = true;

    ssid_flash_store.write(wifi_store);

  } 
  else
  {
   
  }
}

void passCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic)
{
  WifiPassStore wifi_store; 
  Serial.print("Characteristic event, written: ");

  if(passwordCharacteristic.value())
  {
    Serial.println(passwordCharacteristic.value());

    passwordCharacteristic.value().toCharArray(wifi_store.password, passwordCharacteristic.valueSize());
    wifi_store.isValid = true;

    pass_flash_store.write(wifi_store);

  } 
  else
  {

  }
}

void tokenCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic)
{
  TokenPassStore token_store;

  Serial.print("Characteristic event, written: ");

  if(tokenCharacteristic.value())
  {
    Serial.println(tokenCharacteristic.value());
    tokenCharacteristic.value().toCharArray(token_store.token, tokenCharacteristic.valueSize());
    token_store.isValid = true;

    token_flash_store.write(token_store);

  } 
  else
  {

  }
}

void flushCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic)
{

  Serial.print("Characteristic event, written: ");

  if(flushCharacteristic.value())
  {
    Serial.println(flushCharacteristic.value());
    NewFlushDuration = flushCharacteristic.value().toInt();
    Serial.print("NewFlushDuration ");
    Serial.println(NewFlushDuration);
     
    flush_duration_store.write(NewFlushDuration);

  } 
  else
  {

  }
}




#define SEL_PRESS 3000

#define WIFISSID "Loowatt1234"  //SKYIMQXN
#define PASSWORD "Loowatt1234"  //Pm622PsNgdxD
#define TOKEN "BBFF-qte6ng9nuKC7IVLurLoULJvm3nqQ44"

const char * WifiSSID = WIFISSID;
const char * WifiPass = PASSWORD;
const char * WifiToken = TOKEN;

/*
Pod F1 - BBFF-ispRoe1u1rAcw4MqDygsY9wgAsoEws
Pod F2 - BBFF-QyruaMiqLvnUdIZT0fEy762fL0Z9gD
Pod F3 - BBFF-YI76KK72y8VT6Isi1rwIObbekXS9dV
Pof F4 - BBFF-WP6PBUJbXexjrv3dTqXdhNTicPXojh


Toilet 1- BBFF-patudE0sOdTZatRnUihai4MypSfB28  
Toilet 2- BBFF-qte6ng9nuKC7IVLurLoULJvm3nqQ44
Toilet 3- BBFF-tQiPMhdZAQ7RJ6PH08ld0x5xLLeUoW 

Toilet 4- BBFF-4NxLEewovlEuMKBVOL3z5Hpdwzomut
Toilet 5- BBFF-puh0ETQPz7uIKWlwloQ4tiLs4eflfS
Toilet 6- BBFF-B9tOxf7wRt5qOFDlfluzG76UdhkSJ7

Toilet 7- BBFF-kxzZ5eoGeJCtyYdjREdiumIxKoRBSK
Toilet 8- BBFF-u4RYmThRIDoWif6WyFVGT7Z3hFHb1R
Toilet 9- BBFF-pDmMmH8I7KE1pxOeWlGgNK2W9UHQIR


Test Unit 123 - BBFF-aBsNfySRXeRJICKo4TXf6WjDRcjJ4j

Test Unit 125 - BBFF-RqTIKigU6wwypXiHpuLULbbMrcVVmG
*/


#define MQTT_CLIENT_NAME "ubidotclient"

#define POD_NAME_LABEL "TestPod"

String hostName = "www.google.com";
int pingResult;

char mqttBroker[] = "industrial.api.ubidots.com";

WiFiClient wifi;
PubSubClient client(wifi);

#define MQTT_MAX_PACKET_SIZE 1024

#define MQTT_FILM_LEFT_LABEL "FilmLeftCount"
#define MQTT_PERCENTAGE_LABEL "Percentage Left"
#define MQTT_STATE_LABEL "State"
#define MQTT_JAM_LABEL "Jam"
#define MQTT_REFILL_LABEL "Refill"
#define MQTT_LT_JAM_LABEL "Jam"
#define MQTT_LT_SERVICE_LABEL "Service"
#define MQTT_LT_FILM_LABEL "Film"
#define MQTT_LT_FLUSHES_LABEL "Flushes"

#define MQTT_toilet_type "toilet"  //change to toilet
#define MQTT_sealing_unit "su" //change to SU

Button btn_up(BTN_UP);
Button btn_down(BTN_DOWN);
Button btn_sel(BTN_SEL);
Button btn_mode(BTN_MEN);
Button btn_flush(BTN_FLUSH);
Button btn_rear(BTN_REAR);

const unsigned long MAX_FILM_COUNT = 216000;
unsigned long PREVIOUS_FILM_COUNT;
float actual_film_length_mm = 16000.0;
float usable_film_length_m = 13.5;

T_MenuState MenuState = ME_INITIALISING;

T_TwoBytesData UID_REG;
T_StatusByte STATUS_REG;
T_FourBytesData_Signed FILM_LEFT_REG;
T_MotorControlByte MOTORCTRL_REG;

//eeprom data
T_FourBytesData LT_FLUSHES;
T_TwoBytesData LT_BLOCKAGES;
T_TwoBytesData LT_SERVICES;
T_FourBytesData LT_FILM_USED;

char MAC_ID_STR[14];

byte backendReadByte(T_ADDR ADDR);
unsigned short backendReadShort(T_ADDR ADDR);
signed long backendReadLong(T_ADDR ADDR);

void backendWriteByte(T_ADDR ADDR, byte REG);
void backendWriteFourBytes(T_ADDR ADDR, T_FourBytesData_Signed REG);

U8G2_ST7565_NHD_C12864_F_4W_HW_SPI u8g2(U8G2_R2, LCD_CS, LCD_DC, LCD_RESET);

void initialising_frame();
void normal_frame(byte state, signed long film_left);
void refill_frame(signed long film_left);
void jam_frame(signed long film_left);
void su_frame();
void refill_confirmation_frame();
void jam_confirmation_frame();
void default_frame();
void setup_frame();
void wifi_frame();
void wifi_yes_frame();
void wifi_no_frame();




void pollMenuEvent();
void MenuAction();
void lcdUpdate();
void reconnect();
void disconnect();
void sent_data();
void lifetime_data();
void auto_flush();
void auto_check();


void callback(char *topic, byte *payload, unsigned int length);

// TASKs
Task pollMenuEventTask(100, TASK_FOREVER, &pollMenuEvent);
Task MenuActionTask(20, TASK_FOREVER, &MenuAction);
Task lcdUpdateTask(20, TASK_FOREVER, &lcdUpdate);

//Task autoflushTask(100, TASK_FOREVER, &auto_flush);
//Task checkerTask(10000, TASK_FOREVER, &auto_check);


Task DisconnectionTask(600000, TASK_FOREVER, &disconnect);
Task ReconnectionTask(30000, TASK_FOREVER, &reconnect);  //300000 - 5 mins
Task SendingDataTask(60000, TASK_FOREVER, &sent_data);   //600000 - 10 mins
Task SendingLTDataTask(3600000, TASK_FOREVER, &lifetime_data); //3600000 - 60 mins



Scheduler runner;

bool ShowSu = false;

int j = 0;
int k = 0;
int z = 0;

unsigned long myTime;
bool autoStop = false;


int period = 30000;
unsigned long time_now = 0;

void BluetoothAction()
{
      WiFi.disconnect();
      delay(1000);
      ReconnectionTask.disable();
      SendingDataTask.disable();
      SendingLTDataTask.disable();
      pollMenuEventTask.disable();
      MenuActionTask.disable();
      lcdUpdateTask.disable();
  
      delay(1000);

      if(!BLE.begin()){
        Serial.println("BLE failed!");
        while (1);
      }

      

    BLE.setLocalName("WiFi Setup");
    BLE.setAdvertisedService(BLEWifi);
    BLEWifi.addCharacteristic(ssidCharacteristic);
    BLEWifi.addCharacteristic(passwordCharacteristic);
    BLEWifi.addCharacteristic(flushCharacteristic);
    BLEWifi.addCharacteristic(tokenCharacteristic);
    BLE.addService(BLEWifi);

    BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
    BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

    ssidCharacteristic.setEventHandler(BLEWritten, ssidCharacteristicWritten);
    passwordCharacteristic.setEventHandler(BLEWritten, passCharacteristicWritten);

    flushCharacteristic.setEventHandler(BLEWritten, flushCharacteristicWritten);
    tokenCharacteristic.setEventHandler(BLEWritten, tokenCharacteristicWritten);

    BLE.advertise();
    Serial.println("Waiting for connections");
}


void pollMenuEvent()
{
  btn_up.read();
  btn_down.read();
  btn_sel.read();
  btn_mode.read();
  btn_flush.read();
  btn_rear.read();


  if (btn_up.changed() || btn_down.changed() || btn_sel.changed() || btn_mode.changed() || btn_flush.changed() || btn_rear.changed() || ShowSu)
  {

    //checkerTask.enableIfNot();
   
    STATUS_REG.u_status = backendReadByte(T_STATUS);

    if (STATUS_REG.s_status.state == S_RUN)
    {
      
     if(btn_flush.isPressed())
     {
       digitalWrite(FLUSH_LED, LOW);
     }

     else
     {
       digitalWrite(FLUSH_LED, HIGH);
     }
      
    }
 
    if (STATUS_REG.s_status.film == EMPTY)
    {
      if (MenuState != ME_REFILL_CONFIRMATION)
      {
        MenuState = ME_REFILM;
        STATUS_REG.s_status.state = S_SER;
        backendWriteByte(T_STATUS, STATUS_REG.u_status);
      }
      //Serial.println("Empty");
    }

    if (STATUS_REG.s_status.jam == JAM)
    {
      SendingDataTask.forceNextIteration();
      if (MenuState != ME_JAM_CONFIRMATION)
      {
        MenuState = ME_JAM;
        STATUS_REG.s_status.state = S_SER;
        backendWriteByte(T_STATUS, STATUS_REG.u_status);
      }
      //Serial.println("JAM");
    }

    // check flags for low film and jamm
    MenuActionTask.enable();
  }


  if (btn_sel.pressedFor(SEL_PRESS))
  {
    if (MenuState == ME_SERVICE)
    {
      ShowSu = true;
    }

    if (MenuState == ME_RUN)
    {
      ShowWifi = true;
    }
  }
}


void FlushAction()
{
  MOTORCTRL_REG.u_motorcontrol = backendReadByte(T_MOTORCTRL);

  if (MOTORCTRL_REG.s_motorcontrol.m_state == MSTOP)
  {
    MOTORCTRL_REG.s_motorcontrol.m_state = MBACKWARD;
    MOTORCTRL_REG.s_motorcontrol.flush = MFLUSH;
    backendWriteByte(T_MOTORCTRL, MOTORCTRL_REG.u_motorcontrol);
  }
}

void MoveFowardAction()
{
  MOTORCTRL_REG.u_motorcontrol = backendReadByte(T_MOTORCTRL);

  if (MOTORCTRL_REG.s_motorcontrol.m_state == MSTOP)
  {
    MOTORCTRL_REG.s_motorcontrol.m_state = MFORWARD;
    MOTORCTRL_REG.s_motorcontrol.flush = MNOTFLUSH;
    backendWriteByte(T_MOTORCTRL, MOTORCTRL_REG.u_motorcontrol);
  }
}

void MoveBackwardAction()
{
  MOTORCTRL_REG.u_motorcontrol = backendReadByte(T_MOTORCTRL);

  if (MOTORCTRL_REG.s_motorcontrol.m_state == MSTOP)
  {
    MOTORCTRL_REG.s_motorcontrol.m_state = MBACKWARD;
    MOTORCTRL_REG.s_motorcontrol.flush = MNOTFLUSH;
    backendWriteByte(T_MOTORCTRL, MOTORCTRL_REG.u_motorcontrol);
  }
}

void StopAction()
{
  MOTORCTRL_REG.u_motorcontrol = backendReadByte(T_MOTORCTRL);

  if (MOTORCTRL_REG.s_motorcontrol.m_state != MSTOP)
  {
    MOTORCTRL_REG.s_motorcontrol.m_state = MSTOP;
    MOTORCTRL_REG.s_motorcontrol.flush = MNOTFLUSH;
    backendWriteByte(T_MOTORCTRL, MOTORCTRL_REG.u_motorcontrol);
  }
  FILM_LEFT_REG.u_fourbytes = backendReadLong(T_FILM_LEFT);
}


//############################ AUTO FLUSH STUFF ################################

// void auto_check()
// {
//   STATUS_REG.u_status = backendReadByte(T_STATUS);

//   if (STATUS_REG.s_status.state == S_RUN)
//   {
//     myTime = millis();
//     autoStop = true;
//     autoflushTask.enableIfNot();
//   }

// }

// void auto_flush()
// {
//   if (autoStop)
//   {

//     MoveBackwardAction();

//     if (millis() - myTime >= 500)
//     {
//       //Serial.println("AutoFlush Ended");
//       StopAction();

//       autoStop = false;
//       autoflushTask.disable();
//     }
//     if (btn_up.changed() || btn_down.changed() || btn_sel.changed() || btn_mode.changed() || btn_flush.changed() || btn_rear.changed())
//     {
//       StopAction();
//       autoflushTask.disable();
//     }
//   }
// }

void MenuAction()
{
  static uint32_t timeFlushStarted = 0;
  static bool needStop = false;
  

  
  MenuActionTask.disable();
  switch (MenuState)
  {
  case ME_INITIALISING:
    // need an initialisation action
    break;
  case ME_RUN:

    noTone(BUZZER);

    ///////////////////////////UP///////////////////////////////////////////

    if (btn_up.wasPressed())
    {
      MoveBackwardAction();
    }

    else if (btn_up.wasReleased())
    {
      StopAction();
    }

    ///////////////////////////DOWN/////////////////////////////////////////

    else if (btn_down.wasPressed())
    {
      MoveFowardAction();
    }

    else if (btn_down.wasReleased())
    {
      StopAction();
    }

    else if (btn_sel.wasPressed())
    {


    }
    ///////////////////////////FLUSH/////////////////////////////////////////

    else if (btn_flush.wasPressed())
    {
      FlushAction();
      timeFlushStarted = millis();
    }

    else if (btn_flush.wasReleased())
    {
      if(millis() - timeFlushStarted < 1000)
      {
        needStop = true;
      }
      else
      {
        StopAction();
      }
    }

    ///////////////////////////REAR/////////////////////////////////////////

    else if (btn_rear.wasPressed())
    {
      MoveBackwardAction();
    }

    else if (btn_rear.wasReleased())
    {
      StopAction();
    }

    else if(ShowWifi)
    {
    
      ShowWifi = false;
      MenuState = ME_WIFI_SETUP;

    }

    ///////////////////////////MODE/////////////////////////////////////////
    else if (btn_mode.wasPressed())
    {
      MenuState = ME_SERVICE;
      STATUS_REG.s_status.state = S_SER;
      backendWriteByte(T_STATUS, STATUS_REG.u_status);
      tone(BUZZER, 1000);
      //delay(50);
    }

    if(needStop)
    {
      //if(millis() - timeFlushStarted >= 2000)
      //{
        delay(FlushDuration);
        StopAction();
        needStop = false;
      //}
    }

    break;

  case ME_SERVICE:
    ///////////////////////////UP/////////////////////////////////////////
    //Flush Led is OFF
    noTone(BUZZER); //buzzer is OFF
    //delay(50);

    if (btn_up.wasPressed())
    {
      MoveBackwardAction();
    }

    else if (btn_up.wasReleased())
    {
      StopAction();
    }

    ///////////////////////////DOWN/////////////////////////////////////////
    else if (btn_down.wasPressed())
    {
      MoveFowardAction();
    }

    else if (btn_down.wasReleased())
    {
      StopAction();
    }

    else if (btn_mode.wasPressed())
    {
      MenuState = ME_REFILM;
      tone(BUZZER, 1000);
      //delay(50);
    }

    else if (btn_rear.wasPressed())
    {
      MoveBackwardAction();
    }

    else if (btn_rear.wasReleased())
    {
      StopAction();
    }

    else if (ShowSu)
    {
      ShowSu = false;
      LT_FLUSHES.u_fourbytes = backendReadLong(T_LT_FLUSH);
      LT_FILM_USED.u_fourbytes = backendReadLong(T_LT_FILM);
      LT_BLOCKAGES.u_twobytes = backendReadShort(T_LT_BLOCK);
      LT_SERVICES.u_twobytes = backendReadShort(T_LT_SER);
      MenuState = ME_SUDATA;
    }
    break;

  case ME_REFILM:

    noTone(BUZZER);
    //delay(50);

    if (btn_sel.wasPressed())
    {
      MenuState = ME_REFILL_CONFIRMATION;
      tone(BUZZER, 1000);

    }
    else if (btn_mode.wasPressed())
    {
      MenuState = ME_RUN;
      STATUS_REG.s_status.state = S_RUN;
      backendWriteByte(T_STATUS, STATUS_REG.u_status);
      tone(BUZZER, 1000);
    }

    if (btn_up.wasPressed())
    {
      MoveBackwardAction();
    }

    else if (btn_up.wasReleased())
    {
      StopAction();
    }

    else if (btn_rear.wasPressed())
    {
      MoveBackwardAction();
    }

    else if (btn_rear.wasReleased())
    {
      StopAction();
    }

    break;

  case ME_REFILL_CONFIRMATION:
    //Flush Led is OFF
    noTone(BUZZER);

    if (btn_sel.wasPressed())
    {
      MenuState = ME_RUN;
      STATUS_REG.s_status.state = S_RUN;
      FILM_LEFT_REG.u_fourbytes = MAX_FILM_COUNT;
      backendWriteByte(T_STATUS, STATUS_REG.u_status);
      backendWriteFourBytes(T_FILM_LEFT, FILM_LEFT_REG);
      tone(BUZZER, 1000);
    }
    else if (btn_mode.wasPressed())
    {
      MenuState = ME_REFILM;
    }

    break;

  case ME_SUDATA:
    if (btn_mode.wasPressed())
    {
      MenuState = ME_SERVICE;
    }
    break;

  case ME_JAM:
    //Flush Led is OFF
    if (btn_sel.wasPressed())
    {
      MenuState = ME_JAM_CONFIRMATION;
      tone(BUZZER, 1000);
    }

    else if (btn_up.wasPressed())
    {
      MoveBackwardAction();
    }
    else if (btn_up.wasReleased())
    {
      StopAction();
    }

    ///////////////////////////DOWN/////////////////////////////////////////
    else if (btn_down.wasPressed())
    {
      MoveFowardAction();
    }

    else if (btn_down.wasReleased())
    {
      StopAction();
    }
    break;

   case ME_JAM_CONFIRMATION:
    //Flush Led is OFF
    noTone(BUZZER);

    if (btn_sel.wasPressed())
    {
      MenuState = ME_RUN;
      STATUS_REG.s_status.state = S_RUN;
      backendWriteByte(T_STATUS, STATUS_REG.u_status);
      tone(BUZZER, 1000);
    }
    else if (btn_mode.wasPressed())
    {
      MenuState = ME_JAM;
    }
    break;

   case ME_WIFI_SETUP:

    if(btn_sel.wasPressed())
    {
      MenuState = ME_WIFI_YES;
      isWifiOn = true;
      WifiConfig = isWifiOn;
      wifi_config_store.write(WifiConfig);

      BluetoothAction();
      isBleOn = true;
 
    }

    if(btn_mode.wasPressed())
    {
      MenuState = ME_WIFI_NO;
      isWifiOn = false;
      WifiConfig = isWifiOn;
      wifi_config_store.write(WifiConfig);
    }

    break; 

  default:
    break;
  }

  lcdUpdateTask.enable();
}

void lcdUpdate()
{
  lcdUpdateTask.disable();
  switch (MenuState)
  {
  case ME_INITIALISING:
    break;
  case ME_RUN:
    FILM_LEFT_REG.u_fourbytes = backendReadLong(T_FILM_LEFT);
    normal_frame(STATUS_REG.s_status.state, FILM_LEFT_REG.u_fourbytes);
    break;
  case ME_SERVICE:
    FILM_LEFT_REG.u_fourbytes = backendReadLong(T_FILM_LEFT);
    normal_frame(STATUS_REG.s_status.state, FILM_LEFT_REG.u_fourbytes);
    break;
  case ME_REFILM:
    FILM_LEFT_REG.u_fourbytes = backendReadLong(T_FILM_LEFT);
    refill_frame(FILM_LEFT_REG.u_fourbytes);
    break;
  case ME_JAM:
    FILM_LEFT_REG.u_fourbytes = backendReadLong(T_FILM_LEFT);
    jam_frame(FILM_LEFT_REG.u_fourbytes);
    break;
  case ME_REFILL_CONFIRMATION:
    refill_confirmation_frame();
    break;
  case ME_JAM_CONFIRMATION:
    jam_confirmation_frame();
    break;
  case ME_SUDATA:
    su_frame();
    break;
  case ME_WIFI_SETUP:
    wifi_frame();
    break;
  case ME_WIFI_YES:
    wifi_yes_frame();
    break;
  case ME_WIFI_NO:
    wifi_no_frame();
    break;  
  default:
    default_frame();
    break;
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////

void setup()
{
  Wire.setClock(400000);
  Wire.begin();
  Serial.begin(115200);

  static WifiSSIDStore ssid_store;
  static WifiPassStore pass_store;
  static TokenPassStore token_store;


  pinMode(FLUSH_LED, OUTPUT);
  pinMode(BTN_REAR, INPUT_PULLUP);
  pinMode(BTN_FLUSH, INPUT_PULLUP); //pullup required for touchless button
  pinMode(BUZZER, OUTPUT);

  WifiConfig = wifi_config_store.read();
  ssid_store = ssid_flash_store.read();
  pass_store = pass_flash_store.read();
  NewFlushDuration = flush_duration_store.read();
  //Serial.println(WifiConfig);
  delay(4000);

  if(NewFlushDuration == NULL)
  {
    FlushDuration = 1500;
  }

  else
  {
    FlushDuration = NewFlushDuration;
  }

  Serial.print("Flush duration is: ");
  Serial.println(FlushDuration);


  if(ssid_store.isValid)
  {
    Serial.print("new wifi ssid set is: ");
    Serial.println(ssid_store.ssid);
    WifiSSID = ssid_store.ssid;

  }

  if(pass_store.isValid)
  {
    Serial.print("new wifi ssid set is: ");
    Serial.println(pass_store.password);
    WifiPass = pass_store.password;
  }

  if(token_store.isValid)
  {
    Serial.print("Token is: ");
    Serial.println(token_store.token);
    WifiToken = token_store.token;

  }

  if(WifiConfig == 1)
  {
    WiFi.disconnect();
    delay(250);

  int wifi_retries_count = 0;

  while (WiFi.status() != WL_CONNECTED && wifi_retries_count < 5)
  {
    wifi_retries_count++;
    WiFi.begin(WifiSSID, WifiPass);
    Serial.print(".");
    delay(500);
    Serial.println(WiFi.status());
  }

  byte mac[6];
  WiFi.macAddress(mac);

  sprintf(MAC_ID_STR, "%02X%02X%02X%02X%02X%02X", mac[5], mac[4], mac[3], mac[2], mac[1], mac[0]);

  client.setServer(mqttBroker, 1883);
  client.setCallback(callback);

  }


  UID_REG.u_twobytes = backendReadShort(T_UID);
  STATUS_REG.u_status = backendReadByte(T_STATUS);
  FILM_LEFT_REG.u_fourbytes = backendReadLong(T_FILM_LEFT);

  analogWrite(LCD_LED, 255);

  u8g2.begin();
  initialising_frame();
  delay(1000);
  switch (STATUS_REG.s_status.state)
  {
  case S_RUN:
    MenuState = ME_RUN;
    normal_frame(STATUS_REG.s_status.state, FILM_LEFT_REG.u_fourbytes);
    break;
  case S_SER:
    MenuState = ME_SERVICE;
    normal_frame(STATUS_REG.s_status.state, FILM_LEFT_REG.u_fourbytes);
    break;
  case S_STP:
    /* code */
    break;
  case S_UN:
    /* code */
    break;

  default:
    break;
  }

  btn_up.begin();
  btn_down.begin();
  btn_sel.begin();
  btn_mode.begin();
  btn_flush.begin();

  runner.init();

  runner.addTask(pollMenuEventTask);
  runner.addTask(MenuActionTask);
  runner.addTask(lcdUpdateTask);
  runner.addTask(ReconnectionTask);
  runner.addTask(SendingDataTask);
  runner.addTask(SendingLTDataTask);
  runner.addTask(DisconnectionTask);
  //runner.addTask(autoflushTask);
  //runner.addTask(checkerTask);


  ReconnectionTask.enable();
  SendingDataTask.enable();
  pollMenuEventTask.enable();
  SendingLTDataTask.enable();
  

  if(WiFi.status() != WL_CONNECTED)
  {
    ReconnectionTask.disable();
    SendingDataTask.disable();
    SendingLTDataTask.disable();
  }

  //sodaq_wdt_enable(WDT_PERIOD_4X);

}

void loop()
{
  
  runner.execute();

    // if (sodaq_wdt_flag) 
    // {
    // sodaq_wdt_flag = false;
    // sodaq_wdt_reset();
    // Serial.println("WDT interrupt has been triggered");
    
    // }

    if(isBleOn)
    {
      BLE.poll();
    }
   
}

///////////////////////////////////////////////////////

void initialising_frame()
{
  u8g2.clearBuffer();

  //u8g2.setFont(u8g2_font_9x15B_mf);
  //u8g2.drawStr(15, 10, "Loowatt Ltd");
  u8g2.setFont(u8g2_font_open_iconic_www_1x_t);
  u8g2.drawGlyph(115, 10, 81);
  
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(10, 32, "Connected to:");
  u8g2.setCursor(90, 32);
  u8g2.print(UID_REG.u_twobytes, HEX);
  u8g2.sendBuffer();
  delay(1000);
}

void normal_frame(byte state, signed long film_left)
{
  u8g2.clearBuffer();
  //u8g2.setFont(u8g2_font_9x15B_mf);
  //u8g2.drawStr(30, 10, "Loowatt");

  u8g2.setFont(u8g2_font_6x12_tr);

  u8g2.drawStr(8, 30, "REFILL:");
  u8g2.drawStr(90, 30, "%");

  float mm_film_count = film_left / usable_film_length_m; // arduino can't handle big division it seems
  float percentage = (mm_film_count / actual_film_length_mm) * 100.0;

  if (percentage > 100)
  {
    percentage = 100;
  }
  
  int percentage_int = int(percentage);

  u8g2.setCursor(65, 30);

  if (percentage_int < 100)
  {
    u8g2.print(' ');
  }
  if (percentage_int < 10)
  {
    u8g2.print(' ');
  }

  u8g2.print(percentage_int);

  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(20, 45, "MODE:");
  u8g2.drawStr(65, 45, e_StateTypeToString[state]);

  u8g2.sendBuffer();
}

void refill_frame(signed long film_left)
{

  u8g2.clearBuffer();
  //u8g2.setFont(u8g2_font_9x15B_mf);
  //u8g2.drawStr(30, 10, "Loowatt");

  u8g2.setFont(u8g2_font_6x12_tr);

  u8g2.drawStr(8, 30, "REFILL:");
  u8g2.drawStr(90, 30, "%");

  float mm_film_count = film_left / usable_film_length_m; // arduino can't handle big division it seems
  float percentage = (mm_film_count / actual_film_length_mm) * 100.0;

  int percentage_int = int(percentage);

  u8g2.setCursor(65, 30);

  if (percentage_int < 100)
  {
    u8g2.print(' ');
  }
  if (percentage_int < 10)
  {
    u8g2.print(' ');
  }

  u8g2.print(percentage_int);

  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(20, 45, "MODE:");
  u8g2.drawStr(65, 45, "REFILL");

  u8g2.setFont(u8g2_font_6x12_me);

  u8g2.drawStr(8, 60, "CHANGE REFILL? [OK]");

  u8g2.sendBuffer();
}

void su_frame()
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.setCursor(5, 10);
  u8g2.print("SU-ID:");
  u8g2.setCursor(65, 10);
  u8g2.print(UID_REG.u_twobytes, HEX);


  /*
  u8g2.setCursor(0, 25);
  u8g2.print("LT-Flushes:");
  u8g2.setCursor(80, 25);
  u8g2.print(LT_FLUSHES.u_fourbytes, HEX);
  */

  u8g2.setCursor(0, 25);
  u8g2.print("T-FILM:");
  u8g2.setCursor(65, 25);
  float T_FILM = LT_FILM_USED.u_fourbytes/150;
  u8g2.print(T_FILM, 2);

  u8g2.setCursor(5, 40);
  u8g2.print("L-RFL: ");
  u8g2.setCursor(65, 40);
  u8g2.print(LT_SERVICES.u_twobytes, DEC);

  u8g2.setCursor(5, 55);
  u8g2.print("T-BLK:");
  u8g2.setCursor(65, 55);
  u8g2.print(LT_BLOCKAGES.u_twobytes, DEC);

  u8g2.sendBuffer();
}

void jam_frame(signed long film_left)
{

  u8g2.clearBuffer();
  //u8g2.setFont(u8g2_font_9x15B_mf);
  //u8g2.drawStr(30, 10, "Loowatt");

  u8g2.setFont(u8g2_font_6x12_tr);

  u8g2.drawStr(8, 30, "REFILL:");
  u8g2.drawStr(90, 30, "%");

  float mm_film_count = film_left / usable_film_length_m; // arduino can't handle big division it seems
  float percentage = (mm_film_count / actual_film_length_mm) * 100.0;

  int percentage_int = int(percentage);

  u8g2.setCursor(65, 30);

  if (percentage_int < 100)
  {
    u8g2.print(' ');
  }
  if (percentage_int < 10)
  {
    u8g2.print(' ');
  }

  u8g2.print(percentage_int);

  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(20, 45, "MODE:");
  u8g2.drawStr(65, 45, "BLOCKAGE");

  u8g2.sendBuffer();
}

void refill_confirmation_frame()
{
  u8g2.clearBuffer();
  //u8g2.setFont(u8g2_font_9x15B_mf);
  //u8g2.drawStr(30, 10, "Loowatt");

  u8g2.setFont(u8g2_font_6x12_tr);

  u8g2.drawStr(20, 30, "CONFIRM CHANGE?");
  u8g2.drawStr(50, 45, "[OK]");


  u8g2.sendBuffer();
}

void jam_confirmation_frame()
{
  u8g2.clearBuffer();
  //u8g2.setFont(u8g2_font_9x15B_mf);
  //u8g2.drawStr(30, 10, "Loowatt");

  u8g2.setFont(u8g2_font_6x12_tr);

  u8g2.drawStr(15, 30, "BLOCKAGE CLEARED?");
  u8g2.drawStr(50, 45, "[OK]");


  u8g2.sendBuffer();
}

void default_frame()
{

  u8g2.clearBuffer();

  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(10, 25, "Connection Failed");
  u8g2.sendBuffer();

}

void setup_frame()
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_9x15B_mf);
  u8g2.drawStr(30, 10, "Setup");
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(15, 30, "Reboot after done");
  u8g2.sendBuffer();

}

void wifi_frame()
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x13B_mf);
  u8g2.drawStr(25, 10, "Wifi Setting");
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(5, 25, "Do you want to turn");
  u8g2.drawStr(35, 35, "WIFI ON ?");
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(5, 55, "[NO]");
  u8g2.drawStr(95, 55, "[YES]");
  u8g2.sendBuffer();


}

void wifi_yes_frame()
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x13B_mf);
  u8g2.drawStr(25, 10, "Wifi Setting");
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(20, 30, "Wifi will be ON");
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(3, 55, "After Rebooting");
  u8g2.sendBuffer();
}

void wifi_no_frame()
{
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_7x13B_mf);
  u8g2.drawStr(25, 10, "Wifi Setting");
  u8g2.setFont(u8g2_font_6x12_tr);
  u8g2.drawStr(20, 30, "Wifi is now OFF");
  u8g2.setFont(u8g2_font_6x10_tr);
  u8g2.drawStr(3, 55, "Please Reboot Device");
  u8g2.sendBuffer();
}



//############################### MQTT STUFF ##################################

char payload[100];   //size of payload
char topic[150];     //size of topic
char str_sensor[10]; //size of variable

char LT_payload[100];   //size of payload
char LT_topic[150];     //size of topic
char LT_str_sensor[10]; //size of variable



void callback(char *topic, byte *payload, unsigned int length)
{
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
  Serial.write(payload, length);
  Serial.println(topic);
}

void disconnect()
{
    SendingDataTask.disable();
    SendingLTDataTask.disable();
    ReconnectionTask.disable();
    client.endPublish();
    client.disconnect();

    Serial.println("Restarting Wifi...");

    delay(100);

  if (WiFi.status() != WL_CONNECTED)
  {
    WiFi.begin(WifiSSID, WifiPass);
    delay(1000);
  }

    client.setServer(mqttBroker, 1883);
    client.setCallback(callback);

    SendingDataTask.enable();
    SendingLTDataTask.enable();
}

void reconnect()
{  
  // Loop until we're reconnected
  if (WiFi.status() != WL_CONNECTED)
  {
    if(j < 500)
    {
      
    j = j + 1;
    Serial.println(j);

    WiFi.begin(WifiSSID, WifiPass);
    Serial.print(".");
    delay(250);
    Serial.println(WiFi.status());
    
    client.endPublish();
    client.disconnect();
    delay(50);

    client.setServer(mqttBroker, 1883);
    client.setCallback(callback);

    return;
    }
    else
    {
      WiFi.disconnect();
      DisconnectionTask.enable();
      ReconnectionTask.disable();
      SendingDataTask.disable();
      SendingLTDataTask.disable();
      client.endPublish();
      client.disconnect();
    }
    
  }
  else
  {
    j = 0; //reset wifi no ssid found 

    pingResult = WiFi.ping(hostName);

    if (pingResult >= 0)
    {
      z = 0;
      Serial.print("SUCCESS! RTT = ");
      if (client.connect(MQTT_CLIENT_NAME, WifiToken, ""))
      {
        Serial.println("Handshake with MQTT completed");
        SendingDataTask.enableIfNot();
        SendingLTDataTask.enableIfNot();
        ReconnectionTask.disable();
        DisconnectionTask.disable();
        
      }
      else
      {
        if(k < 10)
        {
          Serial.print("CLIENT Failed, rc= ");
          Serial.print(client.state());

          k = k+1;
        }
        else
        {
          
          k = 0;

          DisconnectionTask.enableIfNot();
          DisconnectionTask.forceNextIteration();
          SendingDataTask.enableIfNot();
          SendingLTDataTask.enableIfNot();
          
        }
        
      }
    }
    else
    {

      if(z < 10)
      {
        Serial.print("PING FAILED! Error code: ");
        Serial.println(pingResult);
        z = z+1;
      }

      else
      {
          WiFi.disconnect();
          ReconnectionTask.disable();
          DisconnectionTask.enable();
          SendingDataTask.disable();
          SendingLTDataTask.disable();
  
      }
    }
  }
}


void sent_data()
{
  float mm_film_count = FILM_LEFT_REG.u_fourbytes / usable_film_length_m; // arduino can't handle big division it seems
  int percentage = (mm_film_count / actual_film_length_mm) * 100.0;

  /* Sending Context Data
  STATUS_REG.u_status = backendReadByte(T_STATUS);

  char toilet_state[10];

  if (STATUS_REG.s_status.state == S_RUN)
  {
    strncpy(toilet_state, "\"Run\"", 10);
  }

  if (STATUS_REG.s_status.state == S_SER)
  {
   strncpy(toilet_state, "\"Service\"", 10);
  }
  */
  
  if (!client.connected())
  {
    // not connect will try again
    ReconnectionTask.enable();
    SendingDataTask.disable();
    SendingLTDataTask.disable();
    return;
  }
  else  //"%s%04X"  //"%s%s%04X_%s"
  {
     sprintf(topic, "%s%s%04X", "/v1.6/devices/",MQTT_toilet_type, UID_REG.u_twobytes);
     sprintf(payload, "{");

     sprintf(payload, "%s\"%s\": %ld", payload, MQTT_FILM_LEFT_LABEL, FILM_LEFT_REG.u_fourbytes); // Cleans the payload

     sprintf(payload, "%s, ", payload);
     sprintf(payload, "%s\"%s\": %d", payload, MQTT_PERCENTAGE_LABEL, percentage); // State payload

     sprintf(payload, "%s, ", payload);
     sprintf(payload, "%s\"%s\": %d", payload, MQTT_STATE_LABEL, STATUS_REG.s_status.state); // State payload
    
     //sprintf(payload,  "%s\"%s\": {\"value\": %d, \"context\":{\"state\": %s}}" , payload, MQTT_STATE_LABEL, STATUS_REG.s_status.state, toilet_state); //Add Context 

     sprintf(payload, "%s, ", payload);
     sprintf(payload, "%s\"%s\": %d", payload, MQTT_JAM_LABEL, STATUS_REG.s_status.jam); // Jam payload

     sprintf(payload, "%s}", payload);

    Serial.println("Publishing data to Ubidots Cloud");
    Serial.print(topic);
    Serial.println(payload);
    client.publish(topic, payload);
    client.flush();
    client.loop();
   
  }
}

void lifetime_data()
{
  LT_FLUSHES.u_fourbytes = backendReadLong(T_LT_FLUSH);
  LT_FILM_USED.u_fourbytes = backendReadLong(T_LT_FILM);
  LT_BLOCKAGES.u_twobytes = backendReadShort(T_LT_BLOCK);
  LT_SERVICES.u_twobytes = backendReadShort(T_LT_SER);

  if (!client.connected())
  {
    //do nothing, because live data interval would be the same at some point.

    // not connect will try again
    //ReconnectionTask.enable();
    //SendingDataTask.disable();
    //SendingLTDataTask.disable();;
    //return;
  }
  else
  {
    sprintf(LT_topic, "%s%s%04X", "/v1.6/devices/",MQTT_sealing_unit, UID_REG.u_twobytes);
    sprintf(LT_payload, "{");

    sprintf(LT_payload, "%s\"%s\": %ld", LT_payload, MQTT_LT_FILM_LABEL, LT_FILM_USED.u_fourbytes); // Cleans the payload

    sprintf(LT_payload, "%s, ", LT_payload);
    sprintf(LT_payload, "%s\"%s\": %d", LT_payload, MQTT_LT_SERVICE_LABEL, LT_SERVICES.u_twobytes); // State payload

    sprintf(LT_payload, "%s, ", LT_payload);
    sprintf(LT_payload, "%s\"%s\": %d", LT_payload, MQTT_LT_JAM_LABEL, LT_BLOCKAGES.u_twobytes); // Jam payload

    sprintf(LT_payload, "%s, ", LT_payload);
    sprintf(LT_payload, "%s\"%s\": %ld", LT_payload, MQTT_LT_FLUSHES_LABEL, LT_FLUSHES.u_fourbytes); // Refill payload

    sprintf(LT_payload, "%s}", LT_payload);

    Serial.println("Publishing data to Ubidots Cloud");
    Serial.print(LT_topic);
    Serial.println(LT_payload);
    client.publish(LT_topic, LT_payload);
    client.flush();
    client.loop();
    
  }
}
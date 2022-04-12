#ifndef BLE_DATA_H_
#define BLE_DATA_H_

#include <ArduinoBLE.h>
#include <WiFiNINA.h>
#include <FlashStorage.h>
#include "loos_type.h"
#include "zen_config.h"
#include <Wire.h>
#include "i2c.h"

T_FourBytesData_Signed FILM_LEFT_REG;
//ble data
T_TwoBytesData ACTUAL_FILM_REG;
T_TwoBytesData PPR_MM_REG;
T_TwoBytesData JAM_RPM_REG;


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


bool ShowWifi = false;
bool isWifiOn = false;
int WifiConfig;


FlashStorage(wifi_config_store, bool); 

FlashStorage(ssid_flash_store, WifiSSIDStore);
FlashStorage(pass_flash_store, WifiPassStore);

unsigned long PREVIOUS_FILM_COUNT;

int NewFlushDuration;
int NewMAX_FILM_COUNT;
int NewActual_FILM;
float NewPPR;
int NewJamRpm;

FlashStorage(flush_duration_store, int);
FlashStorage(film_flash_store, int);
FlashStorage(actualfilm_flash_store, int);
FlashStorage(ppr_flash_store, float);
//FlashStorage(rpm_flash_store, int);

bool isBleOn = false;

BLEDevice central;

BLEService BLEWifi("7f364005-1297-477b-bc7b-1274efe4bfdf"); // BLE LED Service
BLEStringCharacteristic ssidCharacteristic("443c72e7-5f41-4a57-8ed4-bccf872cc2dc", 12, 128);
BLEStringCharacteristic passwordCharacteristic("583cccf9-4a77-4be3-974d-a96037a5a04c", 12, 128);
BLEStringCharacteristic flushCharacteristic("24685982-37db-477f-b211-aca9f14471de", 12, 128);  //flush duration  (1000) (1500) (2000)

BLEStringCharacteristic maxfilmCharacteristic("1331f3cc-12e4-49a8-8bec-edea3064f826", 12, 128); //the refill length
BLEStringCharacteristic actualfilmCharacteristic("3f82a7ac-1cce-4e6b-a992-7c4468e64d6f", 12, 128); //the usable flushable refill length
BLEStringCharacteristic pprfilmCharacteristic("6a9c1fc1-6fdb-4fa3-964c-c57c4248f007", 12, 128); //the calibration pulses per mm refill flushed
BLEStringCharacteristic jamCharacteristic("b9a4f3ff-e52f-4ef8-be5b-4fceb07225fc", 12, 128); // the jam detection rpm

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

void maxfilmCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic)
{


  Serial.print("MAX FILM Characteristic event, written: ");

  if(maxfilmCharacteristic.value())
  {
    Serial.println(maxfilmCharacteristic.value());
    MAX_FILM_COUNT = maxfilmCharacteristic.value().toInt();
    Serial.print("Max Film Count = ");
    Serial.println(MAX_FILM_COUNT);

    NewMAX_FILM_COUNT = MAX_FILM_COUNT;

    film_flash_store.write(NewMAX_FILM_COUNT);
    FILM_LEFT_REG.u_fourbytes = MAX_FILM_COUNT;
    backendWriteFourBytes(T_FILM_LEFT, FILM_LEFT_REG);
    
     
    //flush_duration_store.write(NewFlushDuration);

  } 
  else
  {

  }
}

void actualfilmCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic)
{

  Serial.print("ACTUAL FILM Characteristic event, written: ");

  if(actualfilmCharacteristic.value())
  {
    Serial.println(actualfilmCharacteristic.value());
    actual_film_length_mm = actualfilmCharacteristic.value().toInt();
    Serial.print("Actual Film = ");
    Serial.println(actual_film_length_mm);

    NewActual_FILM = actual_film_length_mm;

    actualfilm_flash_store.write(NewActual_FILM);

    ACTUAL_FILM_REG.u_twobytes = actual_film_length_mm;
    backendWriteTwoBytes(T_ACTUAL_FILM, ACTUAL_FILM_REG);
     
    //flush_duration_store.write(NewFlushDuration);

  } 
  else
  {

  }
}

void pprfilmCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic)
{

  Serial.print("PPR mm Characteristic event, written: ");

  if(pprfilmCharacteristic.value())
  {
    Serial.println(pprfilmCharacteristic.value());
    mm_ppr = pprfilmCharacteristic.value().toFloat();
     
    Serial.print("Pulse_mm = ");
    Serial.println(mm_ppr);

    
    NewPPR = mm_ppr;
    ppr_flash_store.write(NewPPR);

    PPR_MM_REG.u_twobytes = mm_ppr;
    backendWriteTwoBytes(T_PPR_MM, PPR_MM_REG);
     
    //flush_duration_store.write(NewFlushDuration);

  } 
  else
  {

  }
}

void jamCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic)
{

  Serial.print("FLUSH Characteristic event, written: ");

  if(jamCharacteristic.value())
  {
    Serial.println(jamCharacteristic.value());
    jam_rpm = jamCharacteristic.value().toInt();
    Serial.print("JamRpm = ");
    Serial.println(jam_rpm);

    NewJamRpm = jam_rpm;

    JAM_RPM_REG.u_twobytes = jam_rpm;
    backendWriteTwoBytes(T_JAM_RPM, JAM_RPM_REG);

  } 
  else
  {

  }
}

void ssidCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic)
{
  WifiSSIDStore wifi_store; 
  Serial.print("SSID Characteristic event, written: ");

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
  Serial.print("PASS Characteristic event, written: ");

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

void flushCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic)
{

  Serial.print("FLUSH Characteristic event, written: ");

  if(flushCharacteristic.value())
  {
    Serial.println(flushCharacteristic.value());
    NewFlushDuration = flushCharacteristic.value().toInt();
    Serial.print("NewFlushDuration = ");
    Serial.println(NewFlushDuration);
     
    flush_duration_store.write(NewFlushDuration);

  } 
  else
  {

  }
}

#endif /* BLE_DATA_H_ */
#ifndef I2C_H_INCLUDED
#define I2C_H_INCLUDED

#include <Arduino.h>
#include "loos_type.h"


byte backendReadByte(T_ADDR ADDR)
{
  Serial.println("<-");
  byte Buff;
  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write(ADDR);
  Serial.println(Wire.endTransmission(true), DEC);

  Wire.requestFrom(SLAVE_ADDRESS, 1);
  Serial.println(Wire.available(), DEC);

  Buff = Wire.read();

  return Buff;
}

unsigned short backendReadShort(T_ADDR ADDR)
{
  Serial.println("<-");
  T_TwoBytesData Buff;
  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write(ADDR);
  Serial.println(Wire.endTransmission(true), DEC);

  Wire.requestFrom(SLAVE_ADDRESS, 2);
  Serial.println(Wire.available(), DEC);

  Buff.s_twobytes.byte_0 = Wire.read();
  Buff.s_twobytes.byte_1 = Wire.read();

  return Buff.u_twobytes;
}

signed long backendReadLong(T_ADDR ADDR)
{
  Serial.println("<-");
  T_FourBytesData Buff;
  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write(ADDR);
  Serial.println(Wire.endTransmission(true), DEC);

  Wire.requestFrom(SLAVE_ADDRESS, 4);
  Serial.println(Wire.available(), DEC);

  Buff.s_fourbytes.byte_0 = Wire.read();
  Buff.s_fourbytes.byte_1 = Wire.read();
  Buff.s_fourbytes.byte_2 = Wire.read();
  Buff.s_fourbytes.byte_3 = Wire.read();

  return Buff.u_fourbytes;
}

void backendWriteByte(T_ADDR ADDR, byte REG)
{
  Serial.println("->");
  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write(ADDR);
  Serial.println(Wire.endTransmission(true), DEC);
  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write(REG);
  Serial.println(Wire.endTransmission(true), DEC);
}

void backendWriteTwoBytes(T_ADDR ADDR, T_TwoBytesData REG)
{
  Serial.println("->");
  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write(ADDR);
  Serial.println(Wire.endTransmission(true), DEC);
  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write(REG.s_twobytes.byte_0);
  Wire.write(REG.s_twobytes.byte_1);
  Serial.println(Wire.endTransmission(true), DEC);
}

void backendWriteFourBytes(T_ADDR ADDR, T_FourBytesData_Signed REG)
{
  Serial.println("->");
  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write(ADDR);
  Serial.println(Wire.endTransmission(true), DEC);
  Wire.beginTransmission(SLAVE_ADDRESS);
  Wire.write(REG.s_fourbytes.byte_0);
  Wire.write(REG.s_fourbytes.byte_1);
  Wire.write(REG.s_fourbytes.byte_2);
  Wire.write(REG.s_fourbytes.byte_3);
  Serial.println(Wire.endTransmission(true), DEC);
}


#endif
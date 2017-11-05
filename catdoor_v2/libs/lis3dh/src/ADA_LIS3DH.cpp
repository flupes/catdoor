/**************************************************************************/
/*!
    @file     Adafruit_LIS3DH.cpp
    @author   K. Townsend / Limor Fried (Adafruit Industries)
    @license  BSD (see license.txt)

    This is a library for the Adafruit LIS3DH Accel breakout board
    ----> https://www.adafruit.com/products/2809

    Adafruit invests time and resources providing this open source code,
    please support Adafruit and open-source hardware by purchasing
    products from Adafruit!

    @section  HISTORY

    v1.0  - First release
*/
/**************************************************************************/

#include "ADA_LIS3DH.h"

/**************************************************************************/
/*!
    @brief  Instantiates a new LIS3DH class in I2C or SPI mode
*/
/**************************************************************************/
// I2C
ADA_LIS3DH::ADA_LIS3DH() : _sensorID(-1) { I2Cinterface = &Wire; }

ADA_LIS3DH::ADA_LIS3DH(TwoWire *Wi) : _sensorID(-1) { I2Cinterface = Wi; }

/**************************************************************************/
/*!
    @brief  Setups the HW (reads coefficients values, etc.)
*/
/**************************************************************************/
bool ADA_LIS3DH::begin(uint8_t i2caddr) {
  _i2caddr = i2caddr;

  // i2c
  I2Cinterface->begin();

  /*
    Serial.println("Debug");

    for (uint8_t i=0; i<0x30; i++) {
    Serial.print("$");
    Serial.print(i, HEX); Serial.print(" = 0x");
    Serial.println(readRegister8(i), HEX);
    }
  */

  /* Check connection */
  uint8_t deviceid = readRegister8(LIS3DH_REG_WHOAMI);
  if (deviceid != 0x33) {
    /* No LIS3DH detected ... return false */
    // Serial.println(deviceid, HEX);
    return false;
  }

  // enable all axes, normal mode
  writeRegister8(LIS3DH_REG_CTRL1, 0x07);

  // DRDY on INT1
  // Need to set the Data Ready Interrupt BEFORE Data Rate
  writeRegister8(LIS3DH_REG_CTRL3, 0x10);

  // High res & BDU enabled
  writeRegister8(LIS3DH_REG_CTRL4, 0x88);

  delay(8);

  // Clear filters
  readRegister8(LIS3DH_REG_REFERENCE);

  // Wait the turn-on time
  delay(8);

  // 400Hz rate
  // setDataRate(LIS3DH_DATARATE_400_HZ);

  // Turn on orientation config
  // writeRegister8(LIS3DH_REG_PL_CFG, 0x40);

  // enable adcs
  // writeRegister8(LIS3DH_REG_TEMPCFG, 0x80);

  /*
  for (uint8_t i = 0; i < 0x30; i++) {
    Serial.print("$");
    Serial.print(i, HEX);
    Serial.print(" = 0x");
    Serial.println(readRegister8(i), HEX);
  }
  */

  return true;
}

void ADA_LIS3DH::read(void) {
  // read x y z at once

  I2Cinterface->beginTransmission(_i2caddr);
  I2Cinterface->write(LIS3DH_REG_OUT_X_L | 0x80);  // 0x80 for autoincrement
  I2Cinterface->endTransmission();

  I2Cinterface->requestFrom(_i2caddr, 6);
  accel[0] = I2Cinterface->read();
  accel[0] |= ((uint16_t)I2Cinterface->read()) << 8;
  accel[1] = I2Cinterface->read();
  accel[1] |= ((uint16_t)I2Cinterface->read()) << 8;
  accel[2] = I2Cinterface->read();
  accel[2] |= ((uint16_t)I2Cinterface->read()) << 8;
}

uint16_t ADA_LIS3DH::getMaxCount(void) {
  uint8_t range = getRange();
  uint16_t divider = 1;
  if (range == LIS3DH_RANGE_16_G)
    divider = 1365;  // different sensitivity at 16g
  if (range == LIS3DH_RANGE_8_G) divider = 4096;
  if (range == LIS3DH_RANGE_4_G) divider = 8190;
  if (range == LIS3DH_RANGE_2_G) divider = 16380;
  return divider;
}

/**************************************************************************/
/*!
    @brief  Read the auxilary ADC
*/
/**************************************************************************/

int16_t ADA_LIS3DH::readADC(uint8_t adc) {
  if ((adc < 1) || (adc > 3)) return 0;
  uint16_t value;

  adc--;

  uint8_t reg = LIS3DH_REG_OUTADC1_L + adc * 2;

  I2Cinterface->beginTransmission(_i2caddr);
  I2Cinterface->write(reg | 0x80);  // 0x80 for autoincrement
  I2Cinterface->endTransmission();
  I2Cinterface->requestFrom(_i2caddr, 2);
  value = I2Cinterface->read();
  value |= ((uint16_t)I2Cinterface->read()) << 8;

  return value;
}

/**************************************************************************/
/*!
    @brief  Set INT to output for single or double click
*/
/**************************************************************************/

void ADA_LIS3DH::setClick(uint8_t c, uint8_t clickthresh, uint8_t timelimit,
                          uint8_t timelatency, uint8_t timewindow) {
  if (!c) {
    // disable int
    uint8_t r = readRegister8(LIS3DH_REG_CTRL3);
    r &= ~(0x80);  // turn off I1_CLICK
    writeRegister8(LIS3DH_REG_CTRL3, r);
    writeRegister8(LIS3DH_REG_CLICKCFG, 0);
    return;
  }
  // else...

  writeRegister8(LIS3DH_REG_CTRL3, 0x80);  // turn on int1 click
  writeRegister8(LIS3DH_REG_CTRL5, 0x08);  // latch interrupt on int1

  if (c == 1)
    writeRegister8(LIS3DH_REG_CLICKCFG, 0x15);  // turn on all axes & singletap
  if (c == 2)
    writeRegister8(LIS3DH_REG_CLICKCFG, 0x2A);  // turn on all axes & doubletap

  writeRegister8(LIS3DH_REG_CLICKTHS, clickthresh);     // arbitrary
  writeRegister8(LIS3DH_REG_TIMELIMIT, timelimit);      // arbitrary
  writeRegister8(LIS3DH_REG_TIMELATENCY, timelatency);  // arbitrary
  writeRegister8(LIS3DH_REG_TIMEWINDOW, timewindow);    // arbitrary
}

uint8_t ADA_LIS3DH::getClick(void) {
  return readRegister8(LIS3DH_REG_CLICKSRC);
}

/**************************************************************************/
/*!
    @brief  Sets the g range for the accelerometer
*/
/**************************************************************************/
void ADA_LIS3DH::setRange(lis3dh_range_t range) {
  uint8_t r = readRegister8(LIS3DH_REG_CTRL4);
  r &= ~(0x30);
  r |= range << 4;
  writeRegister8(LIS3DH_REG_CTRL4, r);
}

/**************************************************************************/
/*!
    @brief  Sets the g range for the accelerometer
*/
/**************************************************************************/
lis3dh_range_t ADA_LIS3DH::getRange(void) {
  /* Read the data format register to preserve bits */
  return (lis3dh_range_t)((readRegister8(LIS3DH_REG_CTRL4) >> 4) & 0x03);
}

/**************************************************************************/
/*!
    @brief  Sets the data rate for the LIS3DH (controls power consumption)
*/
/**************************************************************************/
void ADA_LIS3DH::setDataRate(lis3dh_dataRate_t dataRate) {
  uint8_t ctl1 = readRegister8(LIS3DH_REG_CTRL1);
  ctl1 &= ~(0xF0);  // mask off bits
  ctl1 |= (dataRate << 4);
  writeRegister8(LIS3DH_REG_CTRL1, ctl1);
}

/**************************************************************************/
/*!
    @brief  Sets the data rate for the LIS3DH (controls power consumption)
*/
/**************************************************************************/
lis3dh_dataRate_t ADA_LIS3DH::getDataRate(void) {
  return (lis3dh_dataRate_t)((readRegister8(LIS3DH_REG_CTRL1) >> 4) & 0x0F);
}

/**************************************************************************/
/*!
    @brief  Writes 8-bits to the specified destination register
*/
/**************************************************************************/
void ADA_LIS3DH::writeRegister8(uint8_t reg, uint8_t value) {
  I2Cinterface->beginTransmission((uint8_t)_i2caddr);
  I2Cinterface->write((uint8_t)reg);
  I2Cinterface->write((uint8_t)value);
  I2Cinterface->endTransmission();
}

/**************************************************************************/
/*!
    @brief  Reads 8-bits from the specified register
*/
/**************************************************************************/
uint8_t ADA_LIS3DH::readRegister8(uint8_t reg) {
  uint8_t value;

  I2Cinterface->beginTransmission(_i2caddr);
  I2Cinterface->write((uint8_t)reg);
  I2Cinterface->endTransmission();

  I2Cinterface->requestFrom(_i2caddr, 1);
  value = I2Cinterface->read();
  return value;
}

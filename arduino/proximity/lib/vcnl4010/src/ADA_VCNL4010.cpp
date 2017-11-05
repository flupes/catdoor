#include "Arduino.h"

#ifdef __AVR_ATtiny85__
#include "TinyWireM.h"
#define Wire TinyWireM
#else
#include <Wire.h>
#endif

#include "ADA_VCNL4010.h"

/**************************************************************************/
/*!
    @brief  Instantiates a new VCNL4010 class
*/
/**************************************************************************/
ADA_VCNL4010::ADA_VCNL4010() {}

/**************************************************************************/
/*!
    @brief  Setups the HW
*/
/**************************************************************************/
boolean ADA_VCNL4010::begin(uint8_t addr) {
  _i2caddr = addr;
  Wire.begin();

  uint8_t rev = read8(VCNL4010_PRODUCTID);
  Serial.println(rev, HEX);
  if ((rev & 0xF0) != 0x20) {
    return false;
  }

  // setLEDcurrent(10); keep the default at startup
  // setFrequency(VCNL4010_390K625); default frequency is already 390.625KHz

  // This enable interupt generation at proximity ready...
  // write8(VCNL4010_INTCONTROL, VCNL4010_INT_READY);
  return true;
}

/**************************************************************************/
/*!
    @brief  Get and set the LED current draw
*/
/**************************************************************************/

void ADA_VCNL4010::setLEDcurrent(uint8_t c) {
  if (c > 20) c = 20;
  write8(VCNL4010_IRLED, c);
}

void ADA_VCNL4010::setProximityRate(vcnl4010_rate r) {
  uint8_t b = (uint8_t)r & 0x07;
  write8(VCNL4010_COMMAND, 0);
  delay(1);
  write8(VCNL4010_PROXRATE, b);
  delay(1);
}

uint8_t ADA_VCNL4010::getLEDcurrent(void) { return read8(VCNL4010_IRLED); }

/**************************************************************************/
/*!
    @brief  Get and set the measurement signal frequency
*/
/**************************************************************************/

void ADA_VCNL4010::setModulatorFrequency(vcnl4010_freq f) {
  uint8_t r = read8(VCNL4010_MODTIMING);
  r &= ~(0b00011000);
  r |= f << 3;
  write8(VCNL4010_MODTIMING, r);
}

void ADA_VCNL4010::setLowThreshold(uint16_t low) {
  write8(0x8A, (low & 0xff00) >> 8);
  write8(0x8B, low & 0xff);
}

void ADA_VCNL4010::setHighThreshold(uint16_t high) {
  write8(0x8C, (high & 0xff00) >> 8);
  write8(0x8D, high & 0xff);
}

void ADA_VCNL4010::setProximThresholdInterrupt(uint8_t count) {
  uint8_t v = (count & 0x07) << 5;
  v |= 0x02;
  write8(VCNL4010_INTCONTROL, v);
  delay(1);
}

void ADA_VCNL4010::setAlsThresholdInterrupt(uint8_t count) {
  uint8_t v = (count & 0x07) << 5;
  v |= 0x03;
  write8(VCNL4010_INTCONTROL, v);
}

void ADA_VCNL4010::activateProximityThresholdInterrupt() {
  write8(VCNL4010_COMMAND, 0x03);
  delay(1);
}

void ADA_VCNL4010::setProximReadyInterrupt() {
  write8(VCNL4010_INTCONTROL, 0x08);
}

void ADA_VCNL4010::setAlsReadyInterrupt() { write8(VCNL4010_INTCONTROL, 0x04); }

uint8_t ADA_VCNL4010::readInterruptStatus() { return read8(VCNL4010_INTSTAT); }

void ADA_VCNL4010::clearInterrupt(uint8_t s) { write8(VCNL4010_INTSTAT, s); }

/**************************************************************************/
/*!
    @brief  Get proximity measurement
*/
/**************************************************************************/

uint16_t ADA_VCNL4010::readProximity(void) {
  uint8_t i = read8(VCNL4010_INTSTAT);
  i &= ~0x80;
  write8(VCNL4010_INTSTAT, i);

  write8(VCNL4010_COMMAND, VCNL4010_MEASUREPROXIMITY);
  while (1) {
    // Serial.println(read8(VCNL4010_INTSTAT), HEX);
    uint8_t result = read8(VCNL4010_COMMAND);
    // Serial.print("Ready = 0x"); Serial.println(result, HEX);
    if (result & VCNL4010_PROXIMITYREADY) {
      return read16(VCNL4010_PROXIMITYDATA);
    }
    delay(1);
  }
}

uint16_t ADA_VCNL4010::readAmbient(void) {
  uint8_t i = read8(VCNL4010_INTSTAT);
  i &= ~0x40;
  write8(VCNL4010_INTSTAT, i);

  write8(VCNL4010_COMMAND, VCNL4010_MEASUREAMBIENT);
  while (1) {
    // Serial.println(read8(VCNL4010_INTSTAT), HEX);
    uint8_t result = read8(VCNL4010_COMMAND);
    // Serial.print("Ready = 0x"); Serial.println(result, HEX);
    if (result & VCNL4010_AMBIENTREADY) {
      return read16(VCNL4010_AMBIENTDATA);
    }
    delay(1);
  }
}

/**************************************************************************/
/*!
    @brief  I2C low level interfacing
*/
/**************************************************************************/

// Read 1 byte from the VCNL4000 at 'address'
uint8_t ADA_VCNL4010::read8(uint8_t address) {
  Wire.beginTransmission(_i2caddr);
  Wire.write(address);
  Wire.endTransmission();

  delayMicroseconds(170);  // delay required

  Wire.requestFrom(_i2caddr, (uint8_t)1);
  while (!Wire.available())
    ;

  return Wire.read();
}

// Read 2 byte from the VCNL4000 at 'address'
uint16_t ADA_VCNL4010::read16(uint8_t address) {
  uint16_t data;

  Wire.beginTransmission(_i2caddr);
  Wire.write(address);
  Wire.endTransmission();

  Wire.requestFrom(_i2caddr, (uint8_t)2);
  while (!Wire.available())
    ;
  data = Wire.read();
  data <<= 8;
  while (!Wire.available())
    ;
  data |= Wire.read();

  return data;
}

// write 1 byte
void ADA_VCNL4010::write8(uint8_t address, uint8_t data) {
  Wire.beginTransmission(_i2caddr);
  Wire.write(address);
  Wire.write(data);
  Wire.endTransmission();
}

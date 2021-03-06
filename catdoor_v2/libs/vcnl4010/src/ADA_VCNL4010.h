/**************************************************************************/
/*!
    @file     Adafruit_VCNL4010.h
    @author   K. Townsend (Adafruit Industries)
        @license  BSD (see license.txt)

        This is a library for the Adafruit VCNL4010 Temp Sensor breakout board
        ----> http://www.adafruit.com/products/1782

        Adafruit invests time and resources providing this open source code,
        please support Adafruit and open-source hardware by purchasing
        products from Adafruit!

        @section  HISTORY

    v1.0  - First release
    2017-10: Extended to support proximity interrupts

*/
/**************************************************************************/

#include "Arduino.h"

// the i2c address
#define VCNL4010_I2CADDR_DEFAULT 0x13

// commands and constants
#define VCNL4010_COMMAND 0x80
#define VCNL4010_PRODUCTID 0x81
#define VCNL4010_PROXRATE 0x82
#define VCNL4010_IRLED 0x83
#define VCNL4010_AMBIENTPARAMETER 0x84
#define VCNL4010_AMBIENTDATA 0x85
#define VCNL4010_PROXIMITYDATA 0x87
#define VCNL4010_INTCONTROL 0x89
#define VCNL4010_PROXINITYADJUST 0x8A
#define VCNL4010_INTSTAT 0x8E
#define VCNL4010_MODTIMING 0x8F

typedef enum {
  VCNL4010_3M125 = 3,
  VCNL4010_1M5625 = 2,
  VCNL4010_781K25 = 1,
  VCNL4010_390K625 = 0,
} vcnl4010_freq;

typedef enum {
  VCNL4010_1_95Hz = 0,
  VCNL4010_3_90625Hz = 1,
  VCNL4010_7_815Hz = 2,
  VCNL4010_16_625Hz = 3,
  VCNL4010_31_25Hz = 4,
  VCNL4010_62_5Hz = 5,
  VCNL4010_125Hz = 6,
  VCNL4010_250Hz = 7
} vcnl4010_rate;

#define VCNL4010_MEASUREAMBIENT 0x10
#define VCNL4010_MEASUREPROXIMITY 0x08
#define VCNL4010_AMBIENTREADY 0x40
#define VCNL4010_PROXIMITYREADY 0x20

class ADA_VCNL4010 {
 public:
  ADA_VCNL4010();
  boolean begin(uint8_t a = VCNL4010_I2CADDR_DEFAULT);

  uint8_t getLEDcurrent(void);
  void setLEDcurrent(uint8_t c);

  void setModulatorFrequency(vcnl4010_freq f);
  void setProximityRate(vcnl4010_rate r);
  void setLowThreshold(uint16_t low);
  void setHighThreshold(uint16_t high);
  void setProximThresholdInterrupt(uint8_t count);
  void setAlsThresholdInterrupt(uint8_t count);
  void setProximReadyInterrupt();
  void setAlsReadyInterrupt();
  void activateProximityThresholdInterrupt();

  uint16_t readProximity(void);
  uint16_t readAmbient(void);
  uint8_t readInterruptStatus(void);
  void clearInterrupt(uint8_t s);

 private:
  void write8(uint8_t address, uint8_t data);
  uint16_t read16(uint8_t address);
  uint8_t read8(uint8_t address);

  uint8_t _i2caddr;
};

#include "ADA_LIS3DH.h"
#include "ADA_VCNL4010.h"

#include "m0_hf_pwm.h"

#define TEST_ACCEL 1
#define TEST_PROXIM 1
#define TEST_PWM 1
//#define TEST_SOLENOIDS 1

#ifdef TEST_ACCEL
#define USE_SERIAL 1
#define TESTING 1
#endif

#ifdef TEST_PROXIM
#define USE_SERIAL 1
#define TESTING 1
#endif

#ifdef TEST_PWM
#define TESTING 1
#endif

#ifdef TEST_SOLENOIDS
#define TESTING 1
#endif

// Initialize the accelerometer with the default parameter --> use default I2C
// address
ADA_LIS3DH accel_sensor = ADA_LIS3DH();
ADA_VCNL4010 proxim_sensor = ADA_VCNL4010();

void setup() {
#ifdef USE_SERIAL
  Serial.begin(9600);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Testing Catdoor!");
#endif

#if defined(TEST_ACCEL) || !defined(TESTING)
  if (!accel_sensor.begin(
          0x18)) {  // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1)
      ;
  }
  Serial.println("LIS3DH found!");

  accel_sensor.setRange(LIS3DH_RANGE_4_G);
#endif

#if defined(TEST_PROXIM) || !defined(TESTING)
  if (!proxim_sensor.begin()) {
    Serial.println("Sensor not found :(");
    while (1)
      ;
  }
  Serial.println("Found VCNL4010");
#endif

  pwm_configure();
  pwm_set(5, 0);
  pwm_set(6, 0);
  pwm_set(11, 0);
}

void loop() {
#ifdef TEST_ACCEL
  // Serial.println("Reading data...");
  accel_sensor.read();
  // Serial.println("OK");
  Serial.print(accel_sensor.accel[0]);
  Serial.print("\t");
  Serial.print(accel_sensor.accel[1]);
  Serial.print("\t");
  Serial.print(accel_sensor.accel[2]);
  Serial.print("\t");
  Serial.print(accel_sensor.getMaxCount());
  Serial.print("\t");
#endif

#ifdef TEST_PROXIM
  Serial.print(proxim_sensor.readAmbient());
  Serial.print("\t");
  Serial.print(proxim_sensor.readProximity());
#endif

#ifdef USE_SERIAL
  Serial.println();
  delay(200);
#endif

#ifdef TEST_PWM
  static uint16_t t = 0;
  uint16_t v = (uint16_t)(256 * (1.0 + sin(2.0 * 3.141592 * (float)t / 500)));
  //  uint16_t v = (uint16_t)(256+256*(0.5+0.5*sin(2.0*3.141592*(float)t/200)));
  pwm_set(11, v);
  t++;
  delay(10);
#endif

#ifdef TEST_SOLENOIDS
  pwm_set(5, 512);
  delay(500);
  pwm_set(5, 300);
  delay(500);
  pwm_set(6, 512);
  delay(500);
  pwm_set(6, 128);
  delay(4000);
  pwm_set(5, 0);
  pwm_set(6, 0);
  delay(8000);
#endif
}

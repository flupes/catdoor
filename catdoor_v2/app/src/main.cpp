#include "ADA_LIS3DH.h"
#include "ADA_VCNL4010.h"

#include "m0_hf_pwm.h"

// #define TEST_ACCEL 1
#define TEST_PROXIM 1
//#define TEST_PWM 1
// #define TEST_SOLENOIDS 1

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

bool ext_int = false;

void proximThreshold() { ext_int = true; }

void setup() {
#ifdef USE_SERIAL
  Serial.begin(115200);
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
  pinMode(A1, INPUT_PULLUP);
  attachInterrupt(A1, proximThreshold, FALLING);
  delay(1);
  proxim_sensor.setModulatorFrequency(VCNL4010_390K625);
  proxim_sensor.setLEDcurrent(16);
  // proxim_sensor.setProximityRate(VCNL4010_125Hz);
  proxim_sensor.write8(0x80, 0);
  delay(1);
  proxim_sensor.write8(0x82, 4);
  delay(1);
  proxim_sensor.setProximThresholdInterrupt(3);
  proxim_sensor.setLowThreshold(0);
  proxim_sensor.setHighThreshold(2200);
  // Serial.print("Interrupt Control = ");
  // Serial.println(proxim_sensor.read8(0x89));
  // Serial.print("Low Threshold = ");
  // Serial.println(proxim_sensor.read16(0x8A));
  // Serial.print("High Threshold = ");
  // Serial.println(proxim_sensor.read16(0x8C));
  // proxim_sensor.write8(VCNL4010_INTSTAT, 0x0);
  proxim_sensor.write8(VCNL4010_COMMAND, 0x03);
  delay(1);
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
  Serial.print("\n");
  delay(200);
#endif

#ifdef TEST_PROXIM
  // Serial.print(proxim_sensor.readAmbient());
  // Serial.print("\t");
  // Serial.print(proxim_sensor.readProximity());
  if (ext_int) {
    uint8_t s = proxim_sensor.readInterruptStatus();
    Serial.print(s);
    delay(1);
    proxim_sensor.write8(VCNL4010_INTSTAT, s);
    if (s == 1) {
      proxim_sensor.setLowThreshold(2100);
      proxim_sensor.setHighThreshold(5000);
    } else {
      proxim_sensor.setLowThreshold(0);
      proxim_sensor.setHighThreshold(2100);
    }
    delay(1);
    proxim_sensor.write8(VCNL4010_COMMAND, 0x03);
    delay(1);
    Serial.println(" }");
    ext_int = false;
  }
  delay(100);
#endif

#ifdef USE_SERIAL
// Serial.println();
// delay(10);
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

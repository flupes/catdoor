#include "accel.h"

// #define USE_PWM
// #define USE_RTC

#ifdef USE_PWM
#include "ledctrl.h"
#include "m0_hf_pwm.h"
LedCtrl& status_led = LedCtrl::Instance();
#endif

#ifdef USE_RTC
#include "RTCLib.h"
RTC_DS3231 rtc;
#endif

// Initialize the accelerometer with the default parameter
// --> use default I2C address
Accel accel_sensor;

bool ext_int = false;

void accelReady() { accel_sensor.data_ready = true; }

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Testing Accelerometer");

  if (!accel_sensor.begin(
          0x18)) {  // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1)
      ;
  }
  Serial.println("LIS3DH found!");

  // Set data rate
  accel_sensor.setDataRate(LIS3DH_DATARATE_10_HZ);

  // Set accelerometer range
  accel_sensor.setRange(LIS3DH_RANGE_2_G);

  // attach hardware interrupt
  pinMode(A2, INPUT_PULLUP);
  attachInterrupt(A2, accelReady, RISING);
  delay(1);
  accel_sensor.read();

#ifdef USE_PWM
  pwm_configure();
  status_led.pulse(5000);
#endif

#ifdef USE_RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC!");
    while (1)
      ;
  }
#endif
}

void loop() {
  // static unsigned long last = millis();

  if (accel_sensor.data_ready) {
    // Serial.println("Reading data...");
    accel_sensor.process();
    if (accel_sensor.new_state) {
      Serial.println(ACCEL_STATES_NAMES[(uint8_t)accel_sensor.state]);
      accel_sensor.new_state = false;
    }
    // Serial.println("OK");
    Serial.print(accel_sensor.accel[0]);
    Serial.print("\t");
    // Serial.print(accel_sensor.accel[1]);
    // Serial.print("\t");
    Serial.print(accel_sensor.accel[2]);
    Serial.print("\t");
    if (accel_sensor.accel[2] < -400) {
      Serial.print("!!!");
    }
    // Serial.print("maxCount=");
    // Serial.print(accel_sensor.getMaxCount());
    // Serial.print("\t");
    // Serial.print("elapsed=");
    // Serial.print(now - last);
    Serial.println();
    ext_int = false;
  }

#ifdef USE_PWM
  status_led.update();
#endif

#ifdef USE_RTC
  DateTime utc = rtc.now();
  utc = utc + TimeSpan(1);
#endif

  delay(25);
}

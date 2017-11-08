#define USE_SERIAL 1

#include "accel.h"
#include "ledctrl.h"
#include "proxim.h"
#include "rtc.h"
#include "utils.h"

#define USE_RTC
#define USE_ACCEL 1
#define USE_PROXIM 1

static const HourMinute morning(8, 30);
static const HourMinute evening(20, 45);

// global objects...
#ifdef USE_RTC
CatTime rtc;
#endif
#ifdef USE_ACCEL
Accel accel_sensor = Accel();
#endif
#ifdef USE_PROXIM
Proxim proxim_sensor = Proxim();
#endif
#ifdef USE_SOLENOIDS
Solenoids sol_actuators = Solenoids();
#endif
LedCtrl status_led;

// interupts function prototypes
void proximThreshold() { proxim_sensor.new_state = true; }

void accelReady() { accel_sensor.data_ready = true; }

void setup() {
#ifdef USE_SERIAL
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Starting Catdoor app...");
#endif

#ifdef USE_ACCEL
  PRINTLN("Checking for Accelerometer");
  // Accelerometer setup
  if (!accel_sensor.begin(0x18)) {
    PRINTLN("Couldnt start accelerometer!");
    while (1)
      ;
  }
  PRINTLN("LIS3DH found.");

  // configure sensor
  accel_sensor.setDataRate(LIS3DH_DATARATE_10_HZ);
  accel_sensor.setRange(LIS3DH_RANGE_2_G);

  // attach hardware interrupt
  pinMode(A2, INPUT_PULLUP);
  attachInterrupt(A2, accelReady, RISING);
  delay(1);             // for some reason, this delay is CRITICAL!
  accel_sensor.read();  // read reset the signal --> start interrupts
#endif

#ifdef USE_PROXIM
  // Proximity sensor setup
  PRINTLN("Checking for Proximity Sensor...");
  if (!proxim_sensor.begin()) {
    PRINTLN("Proximity sensor not found!");
    while (1)
      ;
  }
  PRINTLN("VCNL4010 found.");

  // configure sensor
  proxim_sensor.setModulatorFrequency(VCNL4010_390K625);
  proxim_sensor.setLEDcurrent(16);
  proxim_sensor.setProximityRate(VCNL4010_62_5Hz);

  // prepare hardware interrupt
  pinMode(A1, INPUT_PULLUP);
  attachInterrupt(A1, proximThreshold, FALLING);
  delay(1);
  proxim_sensor.setProximThresholdInterrupt(3);
  proxim_sensor.setLowThreshold(0);
  proxim_sensor.setHighThreshold(Proxim::THRESHOLD);
  proxim_sensor.activateProximityThresholdInterrupt();
#endif

#ifdef USER_RTC
  if (!rtc.begin()) {
    PRINTLN("Couldn't find RTC!");
    while (1)
      ;
  }
#endif

  pwm_configure();
  status_led.pulse(5000);
}

static const unsigned long DELAY = 20;
static const uint16_t PCYCLES = 500;
Timing proxim_stats("PROXIM", 10);
Timing accel_stats("ACCEL", 100);
Timing rtc_stats("RTC", 2);

void loop() {
  static uint16_t counter = 0;
  static unsigned long accum_process = 0;
  static unsigned long accum_delays = 0;
  static unsigned long last_time = micros();

  unsigned long start = micros();
  if (accel_sensor.data_ready) {
    accel_stats.start();
    accel_sensor.process();
    accel_stats.stop();
    if (accel_sensor.new_state) {
      PRINTLN(ACCEL_STATES_NAMES[(uint8_t)accel_sensor.state]);
      accel_sensor.new_state = false;
    }
  }
  if (proxim_sensor.new_state) {
    proxim_stats.start();
    proxim_sensor.process();
    proxim_stats.stop();
    if (proxim_sensor.state == Proxim::CAT) {
      PRINTLN("CAT");
    } else {
      PRINTLN("CLEAR");
    }
  }

  if ((start - last_time) > 60 * 1000000) {
    rtc_stats.start();
    HourMinute hm = rtc.localTime();
    rtc_stats.stop();
    PRINT("Current Time = ");
    PRINT(hm.hour);
    PRINT(":");
    PRINTLN(hm.minute);
    last_time = start;
  }

  status_led.update();
  unsigned long stop = micros();
  unsigned long elapsed = stop - start;
  accum_process += elapsed;
  if (elapsed / 1000 > DELAY) {
    PRINTLN("!!! allocated loop time exceeded!!!");
  } else {
    unsigned int sleep_time = DELAY - elapsed / 1000;
    delay(sleep_time);
    accum_delays += sleep_time;
  }
  counter++;
  if (counter > PCYCLES - 1) {
    PRINT("average loop time = ");
    PRINT(accum_process / PCYCLES);
    PRINT(" us - ");
    PRINT("average sleep time = ");
    PRINT(accum_delays / PCYCLES);
    PRINTLN(" ms");
    accum_process = 0;
    accum_delays = 0;
    counter = 0;
  }
}

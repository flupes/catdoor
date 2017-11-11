#define USE_SERIAL 1

#include "RTClib.h"
#include "accel.h"
#include "daytimes.h"
#include "ledctrl.h"
#include "proxim.h"
#include "solenoids.h"
#include "utils.h"

// global objects...
RTC_DS3231 rtc;
Accel accel_sensor;
Proxim proxim_sensor;
Solenoids sol_actuators;
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

  //
  // Accelerometer setup
  //
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

  //
  // Proximity sensor setup
  //
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

  // RTC
  if (!rtc.begin()) {
    PRINTLN("Couldn't find RTC!");
    while (1)
      ;
  }

  // HF PWM
  pwm_configure();

  // start in unkown state
  status_led.flash(300, 600);
}

static const unsigned long PERIOD = 20;

void loop() {
  static bool cat_exiting = false;
  static bool daylight = false;
  static unsigned long last_time = millis();
  static unsigned long action_time = millis();

  unsigned long now = millis();
  if ((now - last_time) > 30 * 1000) {
    // we are up to the half minute
    DateTime utc = rtc.now();
    DateTime local = utc.getLocalTime(TIME_ZONE_OFFSET);
    uint16_t day = local.yDay();
    if (day == 366) day = 1;  // We will be one day offset for all leap years...
    DateTime sunrise(local.year(), local.month(), local.day(),
                     sunset_sunrise_times[day - 1][0],
                     sunset_sunrise_times[day - 1][1], 0, TIME_ZONE_OFFSET);
    DateTime sunset(local.year(), local.month(), local.day(),
                    sunset_sunrise_times[day - 1][2],
                    sunset_sunrise_times[day - 1][3], 0, TIME_ZONE_OFFSET);

    if (sunrise < local && local < sunset) {
      if (!daylight) {
        daylight = true;
        status_led.pulse(5000);
      }
    } else {
      daylight = false;
      status_led.flash(80, 2900);
    }
    last_time = now;
  }

  // Process accelerometer
  if (accel_sensor.data_ready) {
    accel_sensor.process();
    if (accel_sensor.new_state) {
      PRINTLN(ACCEL_STATES_NAMES[(uint8_t)accel_sensor.state]);
      accel_sensor.new_state = false;
    }
  }

  // Check if cat is in front of door
  if (proxim_sensor.new_state) {
    proxim_sensor.process();
    if (proxim_sensor.state == Proxim::CAT) {
      PRINTLN("CAT");
      if (daylight && accel_sensor.state == Accel::CLOSED) {
        // Retract the door locks!
        sol_actuators.open();
      }
    } else {
      PRINTLN("CLEAR");
    }
  }

  // Mark that cat is going out (to release the solenoids later)
  if (sol_actuators.state == Solenoids::ON &&
      accel_sensor.state == Accel::OPEN_OUT) {
    cat_exiting = true;
  }

  // Release solenoids if cat finished exiting
  if (cat_exiting && accel_sensor.state == Accel::CLOSED &&
      sol_actuators.state == Solenoids::ON) {
    sol_actuators.release();
    cat_exiting = false;
  }

  // Detect jammed condition and try to resolve it
  if (accel_sensor.state == Accel::JAMMED &&
      sol_actuators.state == Solenoids::OFF) {
    sol_actuators.open(true);
    action_time = millis();
  }
  // Release solenoids after an un-jamming operation
  if (accel_sensor.state == Accel::JAMMED &&
      sol_actuators.state == Solenoids::ON) {
    if ((millis() - action_time) > 2000) {
      sol_actuators.release();
    }
  }

  // Go through the solenoid state machine
  sol_actuators.process();

  // Update status LED (flash or pulse)
  status_led.update();

  // Loop is variable period, but we do not really care...
  delay(PERIOD);
}

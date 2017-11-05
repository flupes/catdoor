#define USE_SERIAL 1
#include "catdoor.h"

#define USE_ACCEL 1
#define USE_PROXIM 1

Solenoids *Solenoids::instance = 0;
// global objects...
Accel accel_sensor = Accel();
Proxim proxim_sensor = Proxim();
Solenoids sol_actuators = Solenoids();

// interupts function prototypes
void proximThreshold() { proxim_sensor.callback(); }

void accelReady() {
  // PRINTLN("DRDY");
  accel_sensor.callback();
}

void setup() {
#ifdef USE_SERIAL
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
#endif

#ifdef USE_ACCEL
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
}

void loop() {
  static bool cat_exiting = false;
  static unsigned long action_time = millis();

  if (proxim_sensor.new_state) {
    proxim_sensor.process();
    if (proxim_sensor.state == Proxim::CAT) {
      if (accel_sensor.state == Accel::CLOSED) {
        sol_actuators.open();
      }
    }
  }

  if (accel_sensor.new_state) {
    accel_sensor.process();
  }

  if (sol_actuators.state == Solenoids::ON &&
      accel_sensor.state == Accel::OPEN_OUT) {
    cat_exiting = true;
  }
  if (cat_exiting && accel_sensor.state == Accel::CLOSED &&
      sol_actuators.state == Solenoids::ON) {
    sol_actuators.release();
    cat_exiting = false;
  }

  if (accel_sensor.state == Accel::JAMMED &&
      sol_actuators.state == Solenoids::OFF) {
    sol_actuators.open(true);
    action_time = millis();
  }
  if (accel_sensor.state == Accel::JAMMED &&
      sol_actuators.state == Solenoids::ON) {
    if ((millis() - action_time) > 2000) {
      sol_actuators.release();
    }
  }

  sol_actuators.process();

  delay(20);
}

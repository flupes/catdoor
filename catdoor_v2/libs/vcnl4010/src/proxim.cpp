#include "proxim.h"

volatile bool Proxim::new_state = false;

void Proxim::init() {
  // configure sensor
  setModulatorFrequency(VCNL4010_390K625);
  setLEDcurrent(16);
  setProximityRate(VCNL4010_62_5Hz);

  // prepare hardware interrupt
  pinMode(A1, INPUT_PULLUP);
  attachInterrupt(A1, proximThreshold, FALLING);
  delay(1);

  setProximThresholdInterrupt(3);
  setLowThreshold(0);
  setHighThreshold(CAT_THRESHOLD);

  // Serial.print("Interrupt Control = ");
  // Serial.println(proxim_sensor.read8(0x89));
  // Serial.print("Low Threshold = ");
  // Serial.println(proxim_sensor.read16(0x8A));
  // Serial.print("High Threshold = ");
  // Serial.println(proxim_sensor.read16(0x8C));
  // proxim_sensor.write8(VCNL4010_INTSTAT, 0x0);
  activateProximityThresholdInterrupt();
  delay(1);
}

void Proxim::process() {
  istatus = readInterruptStatus();
  delay(1);
  PRINT(istatus);
  PRINT(" - ");
  clearInterrupt(istatus);
  PRINTLN(readProximity());
  if (istatus == 1) {
    setLowThreshold(CLEAR_THRESHOLD);
    setHighThreshold(65535);
    state = CAT;
  } else {
    setLowThreshold(0);
    setHighThreshold(CAT_THRESHOLD);
    state = CLEAR;
  }
  delay(1);
  activateProximityThresholdInterrupt();
  new_state = false;
}

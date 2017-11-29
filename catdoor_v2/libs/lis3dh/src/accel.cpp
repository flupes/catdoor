#include "accel.h"

Accel::Accel(void)
    : ADA_LIS3DH(), data_ready_(false), new_state_(false), state_(AJAR_OUT) {
  calibration_mrad_ = 0;
  current_mrad_ = 0;
  for (size_t i = 0; i < NUMBER_OF_STATES; i++) {
    states_counter_[i] = 0;
  }
}

int16_t Accel::computeAngle() {
  // Horizontal:
  // Zaccel > 0 --> open IN
  // Zaccel < 0 --> open OUT

  // After considering carefully fast methods to compute atan2
  // with alternate floating point math or lookup table, measurement
  // on the target micropressor (Cortex M0) show that it takes
  // 0.32ms to compute an atan2. This is enourmous, but well
  // below our target of 25ms for the main loop timing.
  // So we allow ourselve some luxyry of trigonometric function
  // using doubles (but we store the result in milliradian)!
  current_mrad_ =
      round(1000.0 * atan2(accel[2], -accel[0])) - calibration_mrad_;
  return current_mrad_;
}

void Accel::evaluateState(state_t s) {
  if (s != state_) {
    for (size_t i = 0; i < NUMBER_OF_STATES; i++) {
      if (i == s) {
        states_counter_[i]++;
      } else {
        states_counter_[i] = 0;
      }
    }
    if (states_counter_[s] > STATE_DEBOUNCE_COUNT) {
      state_ = s;
      new_state_ = true;
      PRINT(ACCEL_STATES_NAMES[(uint8_t)state_]);
      PRINT(" acc_h=");
      PRINT(accel[2]);
      PRINT(" acc_v=");
      PRINT(accel[0]);
      PRINT(" angle=");
      PRINT(0.18 * current_mrad_ / 3.141592);
      PRINTLN("deg");
    }
  }
}

void Accel::calibrate() {
  static const size_t loops = 20;
  // static unsigned long start = millis();
  calibration_mrad_ = 0;
  int16_t angles = 0;
  for (size_t i = 0; i < loops; i++) {
    while (!data_ready_) {
      delay(10);
    }
    // unsigned long now = millis();
    read();
    data_ready_ = false;
    // PRINT("elapsed: ");
    // PRINTLN(now - start);
    angles += computeAngle();
    delay(50);
  }
  calibration_mrad_ = angles / loops;
  PRINT("Calibration of vertical angle = ");
  PRINTLN(0.180 * calibration_mrad_ / 3.141592);
}

void Accel::process() {
  // read get the lastest data and at the same time clears the interupt signal
  read();
  data_ready_ = false;

  // Compute the door angle
  computeAngle();

  // Evaluate the current door state
  if (abs(current_mrad_) < CLOSED_TOLERANCE) {
    evaluateState(CLOSED);
    return;
  }
  if (current_mrad_ > OPENED_ANGLE) {
    evaluateState(OPEN_IN);
    return;
  }
  if (current_mrad_ < -OPENED_ANGLE) {
    evaluateState(OPEN_OUT);
    return;
  }
  if (current_mrad_ >= CLOSED_TOLERANCE) {
    evaluateState(AJAR_IN);
    return;
  }
  if (current_mrad_ <= -CLOSED_TOLERANCE) {
    evaluateState(AJAR_OUT);
    return;
  }
  PRINTLN("State UNDEFINED: we should never get there!");
}

#include "solenoids.h"
#include "m0_hf_pwm.h"

#include <Arduino.h>

// Delays in us on when to change the pwm on the solenoids:
// first line: normal opening
// second line: unjamming opening
// Solenoids are applied the following PWM:
// 1 - Both Solenoids with 0 PWM --> OFF
// 2 - Full PWM on Solenoid 1 --> MAX_A
// 3 - Reduced PWM on Solenoid 1 --> LOW_A
// 4 - Full PWM on Solenoid 2 --> MAX_B
// 5 - Reduced PWM on Solenoid 2 --> ON
// column 1: delay between MAX_A and LOW_A
// column 2: delay between MAX_A and MAX_B
// column 3: delay between MAX_A and STAY_B
static unsigned long retract_times_us[2][3] = {{120000L, 150000L, 270000L},
                                               {240000L, 300000L, 540000L}};

void Solenoids::open() {
  if (state_ == OFF || unjamming_) {
    // Alternate which solenoid trigger first
    if (flip_sides_) {
      pin_a_ = PIN_B;
      pin_b_ = PIN_A;
      flip_sides_ = false;
    } else {
      pin_a_ = PIN_A;
      pin_b_ = PIN_B;
      flip_sides_ = true;
    }
    start_time_ = micros();
    state_ = MAX_A;
    pwm_set(pin_a_, 512);
  }
}

void Solenoids::release() {
  pwm_set(PIN_A, 0);
  pwm_set(PIN_B, 0);
  state_ = OFF;
}

void Solenoids::unjam() {
  unjamming_ = 1;
  open();
}

// Go through the solenoids state machine
// and return true if solenoids were released because a hot condition
void Solenoids::update() {
  unsigned long now = micros();
  switch (state_) {
    case MAX_A:
      if ((now - start_time_) > retract_times_us[unjamming_][0]) {
        pwm_set(pin_a_, 250);
        state_ = LOW_A;
      }
      break;
    case LOW_A:
      if ((now - start_time_) > retract_times_us[unjamming_][1]) {
        pwm_set(pin_b_, 512);
        state_ = MAX_B;
      }
      break;
    case MAX_B:
      if ((now - start_time_) > retract_times_us[unjamming_][2]) {
        pwm_set(pin_b_, 250);
        state_ = ON;
      }
      break;
    case ON:
      if (unjamming_) {
        if ((now - start_time_) > UNJAMMING_DURATION_MS * 1000) {
          release();
          unjamming_ = false;
        }
      }
      if ((now - start_time_) > MAX_ON_DURATION_MS * 1000) {
        release();
        state_ = HOT;
        stop_time_ = now;
      }
      break;
    case HOT:
      if ((now - stop_time_) > COOLDOWN_DURATION_MS * 1000) {
        state_ = OFF;
      }
      break;
    default:
      // nothing
      break;
  }
}

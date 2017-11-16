#ifndef _CATDOOR_SOLENOIDS_H_
#define _CATDOOR_SOLENOIDS_H_

#include "m0_hf_pwm.h"

class Solenoids {
 public:
  static const uint8_t PIN_A = 5;
  static const uint8_t PIN_B = 6;
  static const unsigned long MAX_ON_DURATION = 40000;
  static const unsigned long COOLDOWN_DURATION = 40000;

  typedef enum {
    OFF = 0,
    ON = 1,
    MAX_A = 2,
    STAY_A = 3,
    MAX_B = 4,
    HOT = 5
  } state_t;
  state_t state;
  unsigned long start_time;
  unsigned long stop_time;
  unsigned long factor;
  Solenoids() : state(OFF) {}

  void open(bool boost = false) {
    // PRINT("Solenoids state = ");
    // PRINTLN(state);
    if (boost) {
      factor = 3;
    } else {
      factor = 1;
    }
    if (state == OFF) {
      start_time = millis();
      state = MAX_A;
      pwm_set(PIN_A, 512);
    }
  }

  void release() {
    pwm_set(PIN_A, 0);
    pwm_set(PIN_B, 0);
    state = OFF;
  }

  // Go through the solenoids state machine
  // and return true if solenoids were released because a hot condition
  bool process() {
    unsigned long now = millis();
    bool hot_release = false;
    if (state == MAX_A) {
      if ((now - start_time) > factor * 120) {
        pwm_set(PIN_A, 250);
        state = STAY_A;
      }
    }
    if (state == STAY_A) {
      if ((now - start_time) > factor * 150) {
        pwm_set(PIN_B, 512);
        state = MAX_B;
      }
    }
    if (state == MAX_B) {
      if ((now - start_time) > factor * 300) {
        pwm_set(PIN_B, 250);
        state = ON;
      }
    }
    if (state == ON) {
      if ((now - start_time) > MAX_ON_DURATION) {
        release();
        hot_release = true;
        state = HOT;
        stop_time = now;
      }
    }
    if (state == HOT) {
      // PRINT("state is hot... ");
      // PRINTLN(now - stop_time);
      if ((now - stop_time) > COOLDOWN_DURATION) {
        state = OFF;
      }
    }
    return hot_release;
  }
};

#endif

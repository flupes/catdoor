#ifndef _CATDOOR_SOLENOIDS_H_
#define _CATDOOR_SOLENOIDS_H_

#include <stdint.h>

class Solenoids {
 public:
  static Solenoids& Instance() {
    static Solenoids instance_;
    return instance_;
  }

  Solenoids(Solenoids const&) = delete;
  void operator=(Solenoids const&) = delete;

  static const uint8_t PIN_A = 5;
  static const uint8_t PIN_B = 6;
  static const uint8_t PIN_C = 11;
  /*
    Tests on 2 different solenoids in some sort of enclosure show that
    under 2.8V, it takes ~5min to reach 50 deg C.
    One test was inside a reading glass case (padded) and the other inside
    a ESD safe bag (no air flow).
    2.8V was choosen as a conservative margin of the 5V PWM signal that is
    used at half duty cycle.
    Cooling down after release is a slower process, but the test performed
    are less significant since the enclosure material and air flow would
    play larger role.
  */
  static const unsigned long MAX_ON_DURATION_MS = 120000;
  static const unsigned long COOLDOWN_DURATION_MS = 240000;
  static const unsigned long UNJAMMING_DURATION_MS = 4000;

  typedef enum {
    OFF = 0,
    ON = 1,
    MAX_A = 2,
    LOW_A = 3,
    MAX_B = 4,
    HOT = 5
  } state_t;

  void open();

  void release();

  void unjam();

  // Go through the solenoids state machine
  // and return true if solenoids were released because a hot condition
  void update();

  state_t state() {
    if (state_ == MAX_A || state_ == LOW_A || state_ == MAX_B)
      return ON;
    else
      return state_;
  };

 private:
  Solenoids() : state_(OFF), unjamming_(false), flip_sides_(false) {}

  state_t state_;

  unsigned long start_time_;
  unsigned long stop_time_;
  bool unjamming_;
  bool flip_sides_;
  uint8_t pin_a_;
  uint8_t pin_b_;
};

#endif

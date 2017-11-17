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
  static const unsigned long MAX_ON_DURATION_MS = 20000;
  static const unsigned long COOLDOWN_DURATION_MS = 30000;
  static const unsigned long UNJAMMING_DURATION_MS = 2000;

  typedef enum {
    OFF = 0,
    ON = 1,
    MAX_A = 2,
    STAY_A = 3,
    MAX_B = 4,
    HOT = 5
  } state_t;

  void open();

  void release();

  void unjam();

  // Go through the solenoids state machine
  // and return true if solenoids were released because a hot condition
  void update();

  state_t state() { return state_; };

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

#ifndef _CATDOOR_ACCEL_H_
#define _CATDOOR_ACCEL_H_

#include "ADA_LIS3DH.h"
#include "utils.h"

static const char *ACCEL_STATES_NAMES[] = {"CLOSED", "OPEN_IN", "OPEN_OUT",
                                           "AJAR_IN", "AJAR_OUT"};

class Accel : public ADA_LIS3DH {
 public:
  static const int16_t CLOSED_TOLERANCE = 25;  // milliradian
  static const int16_t OPENED_ANGLE = 655;     // milliradian

  volatile bool data_ready_;
  volatile bool new_state_;

  typedef enum {
    CLOSED = 0,
    OPEN_IN = 1,
    OPEN_OUT = 2,
    AJAR_IN = 3,
    AJAR_OUT = 4
  } state_t;
  volatile state_t state_;

  int16_t current_mrad_;
  int16_t calibration_mrad_;

  Accel(void);

  void calibrate();

  void process();

 protected:
  void evaluateState(state_t s);
  int16_t computeAngle();
  static const size_t NUMBER_OF_STATES = 5;
  static const uint8_t STATE_DEBOUNCE_COUNT = 3;
  uint8_t states_counter_[NUMBER_OF_STATES];
};

#endif

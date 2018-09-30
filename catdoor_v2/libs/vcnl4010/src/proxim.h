#ifndef _CATDOOR_PROXIM_H_
#define _CATDOOR_PROXIM_H_

#include "ADA_VCNL4010.h"
#include "utils.h"

class Proxim : public ADA_VCNL4010 {
 public:
  static const uint16_t CAT_THRESHOLD = 2010;
  static const uint16_t CLEAR_THRESHOLD = 2008;
  typedef enum { CLEAR, CAT } state_t;
  volatile state_t state;
  volatile bool new_state;
  volatile uint8_t istatus;

  Proxim(void) : ADA_VCNL4010(), state(CLEAR), new_state(false) {}

  void process() {
    istatus = readInterruptStatus();
    // PRINTLN(istatus);
    clearInterrupt(istatus);
    // PRINTLN(readProximity());
    if (istatus == 1) {
      setLowThreshold(CLEAR_THRESHOLD);
      setHighThreshold(65535);
      state = CAT;
    } else {
      setLowThreshold(0);
      setHighThreshold(CAT_THRESHOLD);
      state = CLEAR;
    }
    activateProximityThresholdInterrupt();
    new_state = false;
  }
};

#endif

#ifndef _CATDOOR_PROXIM_H_
#define _CATDOOR_PROXIM_H_

#include "ADA_VCNL4010.h"
#include "utils.h"

class Proxim : public ADA_VCNL4010 {
 public:
  static const uint16_t THRESHOLD = 2100;
  typedef enum { CLEAR, CAT } state_t;
  volatile state_t state;
  volatile bool new_state;
  volatile uint8_t istatus;

  Proxim(void) : ADA_VCNL4010(), state(CLEAR), new_state(false) {}

  void process() {
    istatus = readInterruptStatus();
    PRINTLN(istatus);
    clearInterrupt(istatus);
    // PRINTLN(readProximity());
    if (istatus == 1) {
      setLowThreshold(THRESHOLD - 32);
      setHighThreshold(65535);
      state = CAT;
    } else {
      setLowThreshold(0);
      setHighThreshold(THRESHOLD + 32);
      state = CLEAR;
    }
    activateProximityThresholdInterrupt();
    new_state = false;
  }
};

#endif

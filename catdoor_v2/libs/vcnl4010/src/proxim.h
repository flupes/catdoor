#ifndef _CATDOOR_PROXIM_H_
#define _CATDOOR_PROXIM_H_

#include "ADA_VCNL4010.h"
#include "utils.h"

class Proxim : public ADA_VCNL4010 {
 public:
  typedef enum { CLEAR, CAT } state_t;
  static const uint16_t CAT_THRESHOLD = 2010;
  static const uint16_t CLEAR_THRESHOLD = 2008;
  static volatile bool new_state;
  volatile uint8_t istatus;
  volatile state_t state;

  Proxim(void) : ADA_VCNL4010(), state(CLEAR) {}

  static void proximThreshold() { new_state = true; }

  void init();

  void process();
};

#endif

#ifndef _CATDOOR_LEDCTRL_H_
#define _CATDOOR_LEDCTRL_H_

#include <Arduino.h>
#include "VisualFeedback.h"
#include "m0_hf_pwm.h"

class LedCtrl : public VisualFeedback {
  static const uint8_t PIN_L = 11;

 public:
  typedef enum { MANUAL, PULSATING, FLASHING } mode_t;
  mode_t mode;
  uint16_t period;
  uint16_t ontime;
  uint16_t offtime;
  unsigned long last;

  LedCtrl();

  void on();

  void off();

  void pulse(uint16_t period_ms);

  void flash(uint16_t on_time_ms, uint16_t off_time_ms);

  void update();

  void error() { flash(100, 100); }

  void warning() { flash(200, 400); }

  void ok() { pulse(6000); }

  void alive() { flash(80, 6000); }

  static LedCtrl *instance;
};

#endif

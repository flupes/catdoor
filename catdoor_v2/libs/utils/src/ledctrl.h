#ifndef _CATDOOR_LEDCTRL_H_
#define _CATDOOR_LEDCTRL_H_

#include "m0_hf_pwm.h"

class LedCtrl {
  static const uint8_t PIN_L = 11;

 public:
  typedef enum { MANUAL, PULSATING, FLASHING } mode_t;
  mode_t mode;
  uint16_t period;
  uint16_t ontime;
  uint16_t offtime;
  unsigned long last;

  LedCtrl() : mode(MANUAL) {}

  void on() {
    mode = MANUAL;
    pwm_set(PIN_L, 500);
  }
  void off() {
    mode = MANUAL;
    pwm_set(PIN_L, 0);
  }
  void pulse(uint16_t period_ms) {
    period = period_ms;
    mode = PULSATING;
    last = millis();
  }

  void flash(uint16_t on_time_ms, uint16_t off_time_ms) {
    ontime = on_time_ms;
    offtime = off_time_ms;
    mode = FLASHING;
    last = millis();
  }

  void update() {
    static bool on = false;
    if (mode == MANUAL) return;
    unsigned long now = millis();
    if (mode == FLASHING) {
      if (on && ((now - last) > ontime)) {
        pwm_set(PIN_L, 0);
        last = now;
        on = false;
      }
      if (!on && ((now - last) > offtime)) {
        pwm_set(PIN_L, 500);
        last = now;
        on = true;
      }
      return;
    }
    if (mode == PULSATING) {
      unsigned long elapsed = now - last;
      uint16_t v = (uint16_t)(
          32 + 224 * (1.0 + sin(2.0 * 3.141592 * (float)elapsed / period)));
      pwm_set(PIN_L, v);
    }
  }
};

#endif

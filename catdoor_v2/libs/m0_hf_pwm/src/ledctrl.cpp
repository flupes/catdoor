#include "ledctrl.h"
#include "m0_hf_pwm.h"

#include <Arduino.h>

void LedCtrl::on() {
  mode_ = MANUAL;
  pwm_set(PIN_L, 500);
}

void LedCtrl::off() {
  mode_ = MANUAL;
  pwm_set(PIN_L, 0);
}
void LedCtrl::pulse(uint16_t period_ms) {
  if (mode_ == PULSATING && period_ == period_ms) return;
  period_ = period_ms;
  mode_ = PULSATING;
  last_ = millis();
}

void LedCtrl::flash(uint16_t on_time_ms, uint16_t off_time_ms) {
  if (mode_ == FLASHING && ontime_ == on_time_ms && offtime_ == off_time_ms)
    return;
  ontime_ = on_time_ms;
  offtime_ = off_time_ms;
  mode_ = FLASHING;
  last_ = millis();
}

void LedCtrl::update() {
  static bool on = false;
  if (mode_ == MANUAL) return;
  unsigned long now = millis();
  if (mode_ == FLASHING) {
    if (on && ((now - last_) > ontime_)) {
      pwm_set(PIN_L, 0);
      last_ = now;
      on = false;
    }
    if (!on && ((now - last_) > offtime_)) {
      pwm_set(PIN_L, 500);
      last_ = now;
      on = true;
    }
    return;
  }
  if (mode_ == PULSATING) {
    unsigned long elapsed = now - last_;
    uint16_t v = (uint16_t)(
        16 + 240 * (1.0 + sin(2.0 * 3.141592 * (float)elapsed / period_)));
    pwm_set(PIN_L, v);
  }
}

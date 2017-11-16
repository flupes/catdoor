#include "ledctrl.h"

LedCtrl *LedCtrl::instance = 0;

void TC4_Handler()  // Interrupt Service Routine (ISR) for timer TC4
{
  // Check for overflow (OVF) interrupt
  if (TC4->COUNT16.INTFLAG.bit.OVF && TC4->COUNT16.INTENSET.bit.OVF) {
    LedCtrl::instance->update();

    REG_TC4_INTFLAG = TC_INTFLAG_OVF;  // Clear the OVF interrupt flag
  }
}

LedCtrl::LedCtrl() : mode(MANUAL) {
  if (instance != 0) {
    // This is bad...
    while (1)
      ;
  } else {
    instance = this;
  }
}

void LedCtrl::on() {
  mode = MANUAL;
  pwm_set(PIN_L, 500);
}

void LedCtrl::off() {
  mode = MANUAL;
  pwm_set(PIN_L, 0);
}
void LedCtrl::pulse(uint16_t period_ms) {
  period = period_ms;
  mode = PULSATING;
  last = millis();
}

void LedCtrl::flash(uint16_t on_time_ms, uint16_t off_time_ms) {
  ontime = on_time_ms;
  offtime = off_time_ms;
  mode = FLASHING;
  last = millis();
}

void LedCtrl::update() {
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
        16 + 240 * (1.0 + sin(2.0 * 3.141592 * (float)elapsed / period)));
    pwm_set(PIN_L, v);
  }
}

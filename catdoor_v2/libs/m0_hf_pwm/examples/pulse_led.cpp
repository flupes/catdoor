#include "m0_hf_pwm.h"

void setup() {
  pwm_configure();
  pwm_set(5, 0);
  pwm_set(6, 0);
  pwm_set(11, 0);
}

void loop() {
  static uint16_t t = 0;
  uint16_t v = (uint16_t)(256 * (1.0 + sin(2.0 * 3.141592 * (float)t / 500)));
  //  uint16_t v = (uint16_t)(256+256*(0.5+0.5*sin(2.0*3.141592*(float)t/200)));
  pwm_set(11, v);
  t++;
  delay(20);
}

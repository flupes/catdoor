#include "Arduino.h"
#include "m0_hf_pwm.h"

void setup() {
  pwm_configure();
  pwm_set(5, 0);
  pwm_set(6, 0);
}

void loop() {
  pwm_set(5, 512);
  delay(500);
  pwm_set(5, 300);
  delay(500);
  pwm_set(6, 512);
  delay(500);
  pwm_set(6, 128);
  delay(4000);
  pwm_set(5, 0);
  pwm_set(6, 0);
  delay(8000);
}

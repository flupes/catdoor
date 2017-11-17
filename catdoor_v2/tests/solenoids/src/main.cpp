#include <Arduino.h>
#include "VisualFeedback.h"  // just to force the right includes
#include "m0_hf_pwm.h"
#include "solenoids.h"

Solenoids& sol = Solenoids::Instance();

void setup() {
  pwm_configure();

  delay(5000);
  sol.open();
  delay(3000);
  sol.release();

  delay(5000);
  sol.open();
  delay(3000);
  sol.release();

  delay(5000);
  sol.unjam();
  delay(8000);
  sol.unjam();
  delay(8000);

  sol.open();
  delay(Solenoids::MAX_ON_DURATION_MS + 4000);
  sol.open();

  delay(Solenoids::COOLDOWN_DURATION_MS);
  sol.open();
  delay(500);
  sol.release();
  delay(500);
  sol.open();
  delay(500);
  sol.release();

  delay(5000);
  sol.open();
  delay(1000);
  sol.open();
  delay(4000);
  sol.release();

  delay(5000);
  sol.open();
  delay(1000);
  sol.unjam();
  delay(8000);
  sol.release();

  sol.open();
  delay(500);
  sol.release();
  delay(500);
  sol.open();
  delay(500);
  sol.release();
}

void loop() {
  delay(10);
#if 0
  // Old Tests
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
#endif
}

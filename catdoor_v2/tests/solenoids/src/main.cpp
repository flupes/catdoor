#include <Arduino.h>
#include "m0_hf_pwm.h"
#include "solenoids.h"
#include "utils.h"

Solenoids& sol = Solenoids::Instance();

void setup() {
  pwm_configure();
#ifdef USE_SERIAL
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect to start the app
  }
#endif

  PRINTLN("open in 5s");
  delay(5000);
  sol.open();
  delay(3000);
  PRINTLN("release");
  sol.release();

  PRINTLN("open (other side) is 5s");
  delay(5000);
  sol.open();
  delay(3000);
  PRINTLN("release");
  sol.release();

  PRINTLN("unjam is 5s");
  delay(5000);
  sol.unjam();
  PRINTLN("should release in 2s");
  delay(8000);
  PRINTLN("should unjam again, now");
  sol.unjam();
  PRINTLN("should release in 2s");
  delay(8000);

  PRINTLN("open and do not release (hot timer should release)");
  sol.open();
  delay(Solenoids::MAX_ON_DURATION_MS + 4000);
  PRINTLN("try to open again, but should not work (because hot)");
  sol.open();

  delay(Solenoids::COOLDOWN_DURATION_MS);
  PRINTLN("cold now, double open/release");
  sol.open();
  delay(500);
  sol.release();
  delay(500);
  sol.open();
  delay(500);
  sol.release();

  PRINTLN("open in 5s");
  delay(5000);
  sol.open();
  delay(1000);
  PRINTLN("open again, now, but should not do anything since still open");
  sol.open();
  delay(4000);
  PRINTLN("release");
  sol.release();

  PRINTLN("open in 5s");
  delay(5000);
  sol.open();
  delay(1000);
  PRINTLN("unjam now (before release)");
  sol.unjam();
  PRINTLN("should release automatically");
  delay(8000);
  PRINTLN("  before this message");
  sol.release();

  PRINTLN("double open/release");
  sol.open();
  delay(500);
  sol.release();
  delay(500);
  sol.open();
  delay(500);
  sol.release();
  PRINTLN("done.");
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

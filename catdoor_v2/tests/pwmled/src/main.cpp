#include "Arduino.h"

#define USE_SERIAL
#include "ledctrl.h"
#include "m0_hf_pwm.h"
#include "utils.h"

LedCtrl pwmled;

void setup() {
#ifdef USE_SERIAL
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Starting PWM LED");
#endif
  pwm_configure();
  pwmled.pulse(4000);
}

static const unsigned long DELAY = 1000;

Timing stats("LOOP", 10);

void loop() {
  // uint16_t v = (uint16_t)(256 * (1.0 + sin(2.0 * 3.141592 * (float)t /
  // 500)));
  // //  uint16_t v =
  // (uint16_t)(256+256*(0.5+0.5*sin(2.0*3.141592*(float)t/200))); pwm_set(11,
  // v); t++;

  // now update is handled by an ISR inside LedCtrl...
  // pwmled.update();

  stats.start();
  delay(DELAY);
  stats.stop();
}

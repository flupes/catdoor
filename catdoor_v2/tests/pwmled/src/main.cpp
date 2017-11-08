#include "Arduino.h"
#include "m0_hf_pwm.h"

#define USE_SERIAL 1
#include "utils.h"

void setup() {
#ifdef USE_SERIAL
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Starting PWM LED");
#endif
  pwm_configure();
  pwm_set(11, 0);
}

static const unsigned long DELAY = 10;
static const uint16_t PCYCLES = 1000;

void loop() {
  static uint16_t counter = 0;
  static unsigned long last = millis();
  static unsigned long accum = 0;

  // static uint16_t t = 0;
  // uint16_t v = (uint16_t)(256 * (1.0 + sin(2.0 * 3.141592 * (float)t /
  // 500)));
  // //  uint16_t v =
  // (uint16_t)(256+256*(0.5+0.5*sin(2.0*3.141592*(float)t/200))); pwm_set(11,
  // v); t++;

  unsigned long now = millis();
  unsigned long elapsed = now - last;
  accum += elapsed;
  if (elapsed > DELAY) {
    PRINTLN("!!! allocated loop time exceeded!!!");
  } else {
    delay(DELAY - elapsed);
  }
  last = now;
  counter++;
  if (counter > PCYCLES - 1) {
    PRINT("average loop time = ");
    PRINTLN(accum / PCYCLES);
    accum = 0;
    counter = 0;
  }
}

#include "Arduino.h"
#include "watchdog_timer.h"

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect.
  }
  Serial.println("Starting Watchdog Timer test...");

  wdt_configure(8);

  static unsigned long start = millis();
  while (true) {
    delay(100);
    unsigned long now = millis();
    Serial.print("elapsed: ");
    Serial.println(now - start);
  }
}

void loop() {
  // not entering here ;-)
}

#include <Wire.h>
#include "Adafruit_VCNL4010.h"

Adafruit_VCNL4010 vcnl;

void setup() {
  Serial.begin(115200);
  if (! vcnl.begin()) {
    Serial.println("Sensor not found :(");
    while (1);
  }
  Serial.println("Found VCNL4010");
}

void loop() {
  uint16_t proxim;
  unsigned long now;
  now = millis();
  proxim = vcnl.readProximity();
  Serial.print(now);
  Serial.print(" ");
  Serial.println(proxim);
  delay(50);
}

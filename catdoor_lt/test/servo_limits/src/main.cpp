#include <Arduino.h>

#include "m0_hw_servo.h"

M0HwServo servo(M0HwServo::D5, A1);

void setup() {
  Serial.begin(115200);
  int wait_sec = 0;
  while (!Serial) {
    delay(1000);
    wait_sec++;
    if (wait_sec > 12) break;
  }

  analogReadResolution(12);

  servo.SetPeriod(1500);
  delay(1000);
  servo.Configure(450, 0, 2050, 120);
}

void loop() {
  servo.SetDegreeAngle(servo.GetMinDegreeAngle());
  delay(1000);
  uint32_t minFeedback = servo.GetAveragedAnalogFeedback(3);
  Serial.print("MIN Vfeedback: ");
  Serial.println(minFeedback);
  delay(4000);

  servo.SetDegreeAngle(servo.GetMaxDegreeAngle());
  delay(1000);
  uint32_t maxFeedback = servo.GetAveragedAnalogFeedback(3);
  Serial.print("MAX Vfeedback: ");
  Serial.println(maxFeedback);
  delay(4000);

  float measuredvbat = analogRead(A7);
  Serial.print("VBat: ");
  Serial.print(measuredvbat);
  Serial.print(" -> ");
  measuredvbat *= 2;     // we divided by 2, so multiply back
  measuredvbat *= 3.3;   // Multiply by 3.3V, our reference voltage
  measuredvbat /= 4096;  // convert to voltage (12 bits)
  Serial.println(measuredvbat);
}

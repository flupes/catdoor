#include <Arduino.h>

#include <Servo.h>

#include "m0_hw_servo.h"

M0HwServo servo(M0HwServo::D5, A1);

void setup() {
  Serial.begin(115200);
  servo.SetPeriod(1500);
  delay(3000);
  servo.Configure(450, 0, 2050, 120);
}

void loop() {
  // for (int p = 1000; p <= 2000; p += 10) {
  //   servo.SetPeriod(p);
  //   delay(20);
  // }
  // for (int p = 2000; p >= 1000; p -= 10) {
  //   servo.SetPeriod(p);
  //   delay(20);
  // }

  // servo.SetPeriod(2050);
  servo.SetDegreeAngle(servo.GetMinDegreeAngle());
  delay(3000);
  uint32_t minFeedback = servo.GetAnalogFeedback();
  Serial.print("min Vfeedback: ");
  Serial.println(minFeedback);

  servo.SetDegreeAngle(servo.GetMaxDegreeAngle());
  delay(3000);
  uint32_t maxFeedback = servo.GetAnalogFeedback();
  Serial.print("max Vfeedback: ");
  Serial.println(maxFeedback);

  float measuredvbat = analogRead(A7);
  measuredvbat *= 2;     // we divided by 2, so multiply back
  measuredvbat *= 3.3;   // Multiply by 3.3V, our reference voltage
  measuredvbat /= 4096;  // convert to voltage (12 bits)
  Serial.print("VBat: ");
  Serial.println(measuredvbat);
}

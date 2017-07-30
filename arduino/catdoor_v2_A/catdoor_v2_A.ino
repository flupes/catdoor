void setup() {
    pwm_configure();
}

void loop() {
  pwm_set(10, 512);
  delay(1000);
  pwm_set(12, 512);
  delay(3000);
  pwm_set(10, 256);
  delay(1000);
  pwm_set(12, 128);
  delay(5000);
  pwm_set(10, 0);
  pwm_set(12, 0);
  delay(4000);
}

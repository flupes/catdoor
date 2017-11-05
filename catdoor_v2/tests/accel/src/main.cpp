#include "ADA_LIS3DH.h"

// Initialize the accelerometer with the default parameter
// --> use default I2C address
ADA_LIS3DH accel_sensor = ADA_LIS3DH();

bool ext_int = false;

void accelReady() { ext_int = true; }

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Testing Accelerometer - Take 4");

  if (!accel_sensor.begin(
          0x18)) {  // change this to 0x19 for alternative i2c address
    Serial.println("Couldnt start");
    while (1)
      ;
  }
  Serial.println("LIS3DH found!");

  // Set data rate
  accel_sensor.setDataRate(LIS3DH_DATARATE_25_HZ);

  // Set accelerometer range
  accel_sensor.setRange(LIS3DH_RANGE_2_G);

  // attach hardware interrupt
  pinMode(A2, INPUT_PULLUP);
  attachInterrupt(A2, accelReady, RISING);
  delay(1);
  accel_sensor.read();
}

void loop() {
  static unsigned long last = millis();
  static unsigned int counter = 0;

  if (ext_int) {
    unsigned long now = millis();
    // Serial.println("Reading data...");
    accel_sensor.read();
    // Serial.println("OK");
    Serial.print(accel_sensor.accel[0]);
    Serial.print("\t");
    Serial.print(accel_sensor.accel[1]);
    Serial.print("\t");
    Serial.print(accel_sensor.accel[2]);
    Serial.print("\t");
    Serial.print("maxCount=");
    Serial.print(accel_sensor.getMaxCount());
    Serial.print("\t");
    Serial.print("elapsed=");
    Serial.print(now - last);
    Serial.println();
    last = now;
    ext_int = false;
  }
  if (counter == 50) {
    // delay(500);
    counter = 0;
  }
  counter++;
  delay(10);
}

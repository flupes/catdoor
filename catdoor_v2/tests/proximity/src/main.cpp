#include "ADA_VCNL4010.h"

ADA_VCNL4010 proxim_sensor = ADA_VCNL4010();

bool ext_int = false;

void proximThreshold() { ext_int = true; }

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Testing VCNL4010!");

  if (!proxim_sensor.begin()) {
    Serial.println("Sensor not found :(");
    while (1)
      ;
  }
  Serial.println("Found VCNL4010 - Take 3");

  pinMode(A1, INPUT_PULLUP);
  attachInterrupt(A1, proximThreshold, FALLING);
  delay(1);
  proxim_sensor.setModulatorFrequency(VCNL4010_390K625);
  proxim_sensor.setLEDcurrent(16);

  proxim_sensor.setProximityRate(VCNL4010_62_5Hz);
  proxim_sensor.setProximThresholdInterrupt(3);
  proxim_sensor.setLowThreshold(0);
  proxim_sensor.setHighThreshold(2200);
  // Serial.print("Interrupt Control = ");
  // Serial.println(proxim_sensor.read8(0x89));
  // Serial.print("Low Threshold = ");
  // Serial.println(proxim_sensor.read16(0x8A));
  // Serial.print("High Threshold = ");
  // Serial.println(proxim_sensor.read16(0x8C));
  // proxim_sensor.write8(VCNL4010_INTSTAT, 0x0);
  proxim_sensor.activateProximityThresholdInterrupt();
  delay(1);
}

void loop() {
  // Serial.print(proxim_sensor.readAmbient());
  // Serial.print("\t");
  // Serial.print(proxim_sensor.readProximity());
  if (ext_int) {
    uint8_t s = proxim_sensor.readInterruptStatus();
    delay(1);
    Serial.print(s);
    proxim_sensor.clearInterrupt(s);
    if (s == 1) {
      proxim_sensor.setLowThreshold(2100);
      proxim_sensor.setHighThreshold(5000);
    } else {
      proxim_sensor.setLowThreshold(0);
      proxim_sensor.setHighThreshold(2100);
    }
    delay(1);
    proxim_sensor.activateProximityThresholdInterrupt();
    Serial.println();
    ext_int = false;
  }
  delay(100);
}

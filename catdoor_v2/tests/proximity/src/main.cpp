#include "proxim.h"

static const uint16_t CAT_THRESHOLD = 2010;
static const uint16_t CLEAR_THRESHOLD = 2008;

Proxim proxim_sensor;

bool ext_int = false;

void proximThreshold() { proxim_sensor.new_state = true; }

void setup() {
  // init communication
  if (!proxim_sensor.begin()) {
    PRINTLN("Sensor not found :(");
    while (1)
      ;
  }
  PRINTLN("Found VCNL4010");

  proxim_sensor.init();
}

void loop() {
  static int counter = 0;
  // Serial.print(proxim_sensor.readAmbient());
  // Serial.print("\t");
  // if (counter == 20) {
  //   Serial.println(proxim_sensor.readProximity());
  //   counter = 0;
  // }
  counter++;

  if (proxim_sensor.new_state) {
    proxim_sensor.process();
    if (proxim_sensor.state == Proxim::CAT) {
      Serial.println("CAT");
    } else {
      Serial.println("CLEAR");
    }
  }
  delay(20);
}

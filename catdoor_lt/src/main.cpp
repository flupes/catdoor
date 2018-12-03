#include <Arduino.h>

#include <Servo.h>

#include "m0_hw_servo.h"
#include "mqtt_client.h"
#include "netcfg.h"
#include "utils.h"
#include "watchdog_timer.h"

M0HwServo servo(M0HwServo::D5, A1);
WiFiClient wifi_client;
MQTT_Client mqtt_client(wifi_client);

void setup() {
#ifdef USE_SERIAL
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect to start the app
  }
#endif
  servo.SetPeriod(1500);
  delay(3000);
  servo.Configure(450, 0, 2050, 120);

  // wdt_configure(11);

  // Configure pins for Adafruit ATWINC1500 Feather
  WiFi.setPins(8, 7, 4, 2);
  // Check for the presence of the shield
  PRINTLN("Connecting to the Wifi module...");
  if (WiFi.status() == WL_NO_SHIELD) {
    PRINTLN("WiFi shield not present");
    return;  // don't continue
  }

  mqtt_client.connect_wifi(MY_WIFI_SSID, MY_WIFI_PASS);
  mqtt_client.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt_client.connect_client();

  PRINTLN("MQTT client connected.");
  const char* msg = "Catdoor LT started";
  mqtt_client.publish_msg(TOPIC_MESSAGE, msg);
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
  delay(1000);
  mqtt_client.publish_msg(TOPIC_DOORSTATE, "open");
  uint32_t minFeedback = servo.GetAnalogFeedback();
  Serial.print("min Vfeedback: ");
  Serial.println(minFeedback);
  delay(4000);
  wdt_reset();

  servo.SetDegreeAngle(servo.GetMaxDegreeAngle());
  delay(1000);
  mqtt_client.publish_msg(TOPIC_DOORSTATE, "closed");
  uint32_t maxFeedback = servo.GetAnalogFeedback();
  Serial.print("max Vfeedback: ");
  Serial.println(maxFeedback);
  delay(4000);
  wdt_reset();

  float measuredvbat = analogRead(A7);
  measuredvbat *= 2;     // we divided by 2, so multiply back
  measuredvbat *= 3.3;   // Multiply by 3.3V, our reference voltage
  measuredvbat /= 4096;  // convert to voltage (12 bits)
  Serial.print("VBat: ");
  Serial.println(measuredvbat);
}

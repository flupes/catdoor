#include "ADA_LIS3DH.h"

// #define NOISE_PWM
// #define NOISE_WIFI
#define NOISE_RTC

#ifdef NOISE_PWM
#include "ledctrl.h"
LedCtrl status_led;
#endif

#ifdef NOISE_WIFI
#define USE_SERIAL 1
#include "RTCLib.h"
#include "netcfg.h"
WiFiClient wifi_client;
MQTT_Client mqtt_client(wifi_client);
DateTime ltime(2017, 11, 11, 8, 0, 0);
#endif

#ifdef NOISE_RTC
#include "RTCLib.h"
RTC_DS3231 rtc;
#endif

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
  Serial.println("Testing Accelerometer");

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

#ifdef NOISE_PWM
  pwm_configure();
  status_led.pulse(5000);
#endif

#ifdef NOISE_WIFI
  // Configure pins for Adafruit ATWINC1500 Feather
  WiFi.setPins(8, 7, 4, 2);
  // Check for the presence of the shield
  Serial.println("Connecting to the Wifi module...");
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    return;  // don't continue
  }

  connect_wifi(wifi_client, status_led);
  mqtt_client.setServer(MQTT_SERVER, MQTT_PORT);
  if (!mqtt_client.connect_client()) {
    Serial.println("Failed to connect MQTT client!");
    while (1) {
      delay(20);
    }
  }
  Serial.println("MQTT client connected.");
  const char *msg = "Accel with WIFI test started.";
  mqtt_client.publish_timed_msg(ltime, TOPIC_MESSAGE, msg);
#endif

#ifdef NOISE_RTC
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC!");
    while (1)
      ;
  }
#endif
}

void loop() {
  static unsigned long last = millis();

  if (ext_int) {
    // Serial.println("Reading data...");
    accel_sensor.read();
    // Serial.println("OK");
    Serial.print(accel_sensor.accel[0]);
    Serial.print("\t");
    // Serial.print(accel_sensor.accel[1]);
    // Serial.print("\t");
    Serial.print(accel_sensor.accel[2]);
    Serial.print("\t");
    if (accel_sensor.accel[2] < -400) {
      Serial.print("!!!");
    }
    // Serial.print("maxCount=");
    // Serial.print(accel_sensor.getMaxCount());
    // Serial.print("\t");
    // Serial.print("elapsed=");
    // Serial.print(now - last);
    Serial.println();
    ext_int = false;
  }
#ifdef NOISE_PWM
  status_led.update();
#endif

#ifdef NOISE_WIFI
  unsigned long now = millis();
  if (now - last > 2000) {
    mqtt_client.publish_timed_msg(ltime, TOPIC_HEARTBEAT, "ACCEL");
    last = now;
  }
  mqtt_client.loop();
#endif

#ifdef NOISE_RTC
  DateTime utc = rtc.now();
#endif

  delay(25);
}

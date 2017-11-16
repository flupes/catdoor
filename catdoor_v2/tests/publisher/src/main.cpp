#define USE_SERIAL 1

#include "RTClib.h"
#include "daytimes.h"
#include "ledctrl.h"
#include "mqtt_client.h"
#include "netcfg.h"
#include "utils.h"

RTC_DS3231 rtc;
char buffer[64];

LedCtrl status_led;
WiFiClient wifi_client;
MQTT_Client mqtt_client(wifi_client);

void connect_wifi(WiFiClient& wifi_client, LedCtrl& status_led) {
  static uint8_t counter = 0;
  int status = WL_IDLE_STATUS;  // the WiFi radio's status
  // attempt to connect to WiFi network:
  while (status != WL_CONNECTED) {
    if (counter == 0) {
      PRINT("Attempting to connect to WEP network, SSID: ");
      PRINTLN(MY_WIFI_SSID);
      status = WiFi.begin(MY_WIFI_SSID, MY_WIFI_PASS);
      counter = 200;
    }
    counter--;
    status_led.update();
    delay(50);
  }
#ifdef USE_SERIAL
  PRINT("IP address: ");
  IPAddress ip = WiFi.localIP();
  PRINTLN(ip);
#endif
}

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Start MQTT Publisher...");
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC!");
    while (1)
      ;
  }
  // HF PWM
  pwm_configure();

  // start in unkown state
  status_led.flash(200, 100);

  // Configure pins for Adafruit ATWINC1500 Feather
  WiFi.setPins(8, 7, 4, 2);
  // Check for the presence of the shield
  PRINTLN("Connecting to the Wifi Module");
  if (WiFi.status() == WL_NO_SHIELD) {
    PRINTLN("WiFi shield not present");
    return;  // don't continue
  }

  connect_wifi(wifi_client, status_led);
  mqtt_client.setServer(MQTT_SERVER, MQTT_PORT);
}

static const unsigned long PERIOD = 20;

Timing perf("TIMER", 1);

void loop() {
  static int counter = 0;
  static bool daylight = false;
  static unsigned long last_time = millis();

  unsigned long now = millis();
  if ((now - last_time) > 10 * 1000) {
    DateTime utc = rtc.now();
    DateTime local = utc.getLocalTime(TIME_ZONE_OFFSET);
    uint16_t day = local.yDay();
    if (day == 366) day = 1;  // We will be one day offset for all leap years...
    DateTime sunrise(local.year(), local.month(), local.day(),
                     sunset_sunrise_times[day - 1][0],
                     sunset_sunrise_times[day - 1][1], 0, TIME_ZONE_OFFSET);
    DateTime sunset(local.year(), local.month(), local.day(),
                    sunset_sunrise_times[day - 1][2],
                    sunset_sunrise_times[day - 1][3], 0, TIME_ZONE_OFFSET);

    if (true || (sunrise < local && local < sunset)) {
      if (!daylight) {
        daylight = true;
        status_led.pulse(5000);
      }
    } else {
      daylight = false;
      status_led.flash(80, 2900);
    }
    perf.start();
    mqtt_client.publish_heartbeat(local, daylight);
    perf.stop();
    last_time = now;
  }

  // Update status LED (flash or pulse)
  status_led.update();

  // Let the MQTT client do some work
  mqtt_client.loop();

  delay(PERIOD);

  counter++;
}

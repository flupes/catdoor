// #define USE_SERIAL 1

#include "RTClib.h"
#include "accel.h"
#include "daytimes.h"
#include "ledctrl.h"
#include "netcfg.h"
#include "proxim.h"
#include "solenoids.h"
#include "utils.h"

#include <avr/dtostrf.h>

// Where to get the battery voltage
#define VBATPIN A7

// Customization for acceptable daylight times
static const TimeSpan margin_after_sunrise(0, 0, 45, 0);
static const TimeSpan margin_before_sunset(0, 2, 0, 0);

// global objects...
RTC_DS3231 rtc;
Accel accel_sensor;
Proxim proxim_sensor;
Solenoids sol_actuators;
LedCtrl status_led;
WiFiClient wifi_client;
MQTT_Client mqtt_client(wifi_client);

// interupts function prototypes
void proximThreshold() { proxim_sensor.new_state = true; }

void accelReady() { accel_sensor.data_ready = true; }

void setup() {
#ifdef USE_SERIAL
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Starting Catdoor app...");
#endif

  //
  // Accelerometer setup
  //
  if (!accel_sensor.begin(0x18)) {
    PRINTLN("Couldnt start accelerometer!");
    while (1)
      ;
  }
  PRINTLN("LIS3DH found.");

  // configure sensor
  accel_sensor.setDataRate(LIS3DH_DATARATE_10_HZ);
  accel_sensor.setRange(LIS3DH_RANGE_2_G);

  // attach hardware interrupt
  pinMode(A2, INPUT_PULLUP);
  attachInterrupt(A2, accelReady, RISING);
  delay(1);             // for some reason, this delay is CRITICAL!
  accel_sensor.read();  // read reset the signal --> start interrupts

  //
  // Proximity sensor setup
  //
  if (!proxim_sensor.begin()) {
    PRINTLN("Proximity sensor not found!");
    while (1)
      ;
  }
  PRINTLN("VCNL4010 found.");

  // configure sensor
  proxim_sensor.setModulatorFrequency(VCNL4010_390K625);
  proxim_sensor.setLEDcurrent(16);
  proxim_sensor.setProximityRate(VCNL4010_62_5Hz);

  // prepare hardware interrupt
  pinMode(A1, INPUT_PULLUP);
  attachInterrupt(A1, proximThreshold, FALLING);
  delay(1);
  proxim_sensor.setProximThresholdInterrupt(3);
  proxim_sensor.setLowThreshold(0);
  proxim_sensor.setHighThreshold(Proxim::THRESHOLD);
  proxim_sensor.activateProximityThresholdInterrupt();

  // RTC
  if (!rtc.begin()) {
    PRINTLN("Couldn't find RTC!");
    while (1)
      ;
  }

  // HF PWM
  pwm_configure();

  // start in unkown state
  status_led.flash(200, 400);

  // Configure pins for Adafruit ATWINC1500 Feather
  WiFi.setPins(8, 7, 4, 2);
  // Check for the presence of the shield
  PRINTLN("Connecting to the Wifi module...");
  if (WiFi.status() == WL_NO_SHIELD) {
    PRINTLN("WiFi shield not present");
    return;  // don't continue
  }

  connect_wifi(wifi_client, status_led);
  mqtt_client.setServer(MQTT_SERVER, MQTT_PORT);
  if (!mqtt_client.connect_client()) {
    PRINTLN("Failed to connect MQTT client!");
    status_led.flash(100, 100);
    while (1) {
      status_led.update();
      delay(20);
    }
  }
  PRINTLN("MQTT client connected.");
  const char *msg = "Catdoor v2 started";
  DateTime utc_time = rtc.now();
  unsigned int now = millis();
  DateTime local_time = utc_time.getLocalTime(TIME_ZONE_OFFSET);
  mqtt_client.sync_time(local_time, now);
  mqtt_client.publish_timed_msg(now, TOPIC_MESSAGE, msg);
  // start by flashing like in the darkness..
  status_led.flash(80, 2900);
  ;
}

static const unsigned long PERIOD = 25;

void loop() {
  static bool cat_exiting = false;
  static bool jammed_condition = false;
  static bool daylight = false;
  static bool rtc_read = true;
  static unsigned long last_time = millis();
  static unsigned long action_time = millis();
  unsigned long now = millis();
  unsigned long elapsed;
  unsigned long sleep_ms;

  if ((now - last_time) > 10 * 1000) {
    if (rtc_read) {
      // Check RC every 20s
      DateTime ltime = rtc.now().getLocalTime(TIME_ZONE_OFFSET);
      mqtt_client.sync_time(ltime, now);
      uint16_t day = ltime.yDay();
      if (day == 366)
        day = 1;  // We will be one day offset for all leap years...
      DateTime sunrise(ltime.year(), ltime.month(), ltime.day(),
                       sunset_sunrise_times[day - 1][0],
                       sunset_sunrise_times[day - 1][1], 0, TIME_ZONE_OFFSET);
      DateTime sunset(ltime.year(), ltime.month(), ltime.day(),
                      sunset_sunrise_times[day - 1][2],
                      sunset_sunrise_times[day - 1][3], 0, TIME_ZONE_OFFSET);
      DateTime morning = sunrise + margin_after_sunrise;
      DateTime afternoon = sunset - margin_before_sunset;
      if (morning < ltime && ltime < afternoon) {
        if (!daylight) {
          daylight = true;
          status_led.pulse(5000);
          mqtt_client.publish_timed_msg(now, TOPIC_MESSAGE, "UNLOCKED");
        }
      }  // if daylight
      else {
        if (daylight) {
          daylight = false;
          status_led.flash(80, 2900);
          mqtt_client.publish_timed_msg(now, TOPIC_MESSAGE, "LOCKED");
        }
      }  // else daylight
      rtc_read = false;
      mqtt_client.publish_timed_msg(now, TOPIC_HEARTBEAT,
                                    daylight ? "DAYLIGHT" : "DARK");
    } else {
      // alternate with reading the battery voltage
      float measuredvbat = analogRead(VBATPIN);
      measuredvbat *= 2;     // we divided by 2, so multiply back
      measuredvbat *= 3.3;   // Multiply by 3.3V, our reference voltage
      measuredvbat /= 1024;  // convert to voltage
      Serial.print("VBat: ");
      Serial.println(measuredvbat);
      char num[6];
      dtostrf(measuredvbat, 5, 2, num);
      char str[12];
      sprintf(str, "VBAT=%s", num);
      mqtt_client.publish_timed_msg(now, TOPIC_MESSAGE, str);
      rtc_read = true;
    }

    last_time = now;
  }  // if 10s

  // Process accelerometer
  if (accel_sensor.data_ready) {
    accel_sensor.process();
    if (accel_sensor.new_state) {
      PRINTLN(ACCEL_STATES_NAMES[(uint8_t)accel_sensor.state]);
      accel_sensor.new_state = false;
      mqtt_client.publish_timed_msg(
          now, TOPIC_DOOR, ACCEL_STATES_NAMES[(uint8_t)accel_sensor.state]);
    }
  }

  // Check if cat is in front of door
  if (proxim_sensor.new_state) {
    proxim_sensor.process();
    if (proxim_sensor.state == Proxim::CAT) {
      PRINTLN("CAT");
      mqtt_client.publish_timed_msg(now, TOPIC_PROXIMITY, "CAT");
      if (daylight && accel_sensor.state == Accel::CLOSED) {
        // Retract the door locks!
        if (sol_actuators.state == Solenoids::OFF) {
          sol_actuators.open();
          mqtt_client.publish_timed_msg(now, TOPIC_SOLENOIDS, "RETRACTED");
        }
      }
    } else {
      PRINTLN("CLEAR");
      mqtt_client.publish_timed_msg(now, TOPIC_PROXIMITY, "CLEAR");
    }
  }

  // Mark that cat is going out (to release the solenoids later)
  if (sol_actuators.state == Solenoids::ON &&
      accel_sensor.state == Accel::OPEN_OUT) {
    cat_exiting = true;
  }

  // Release solenoids if cat finished exiting
  if (cat_exiting && accel_sensor.state == Accel::CLOSED &&
      sol_actuators.state == Solenoids::ON) {
    sol_actuators.release();
    mqtt_client.publish_timed_msg(now, TOPIC_SOLENOIDS, "RELEASE");
    cat_exiting = false;
  }

  // Detect jammed condition and try to resolve it
  if (accel_sensor.state == Accel::JAMMED &&
      sol_actuators.state == Solenoids::OFF) {
    sol_actuators.open(true);
    mqtt_client.publish_timed_msg(now, TOPIC_SOLENOIDS, "BOOST_RETRACT");
    action_time = now;
    jammed_condition = true;
  }
  // Release solenoids after an un-jamming operation
  if (jammed_condition && sol_actuators.state == Solenoids::ON) {
    if ((now - action_time) > 2000) {
      sol_actuators.release();
      jammed_condition = false;
      mqtt_client.publish_timed_msg(now, TOPIC_SOLENOIDS, "RELEASE_JAMMED");
    }
  }

  // Go through the solenoid state machine
  bool hot_release = sol_actuators.process();
  if (hot_release) {
    jammed_condition = false;
    mqtt_client.publish_timed_msg(now, TOPIC_SOLENOIDS, "RELEASE_HOT");
  }
  // Update status LED (flash or pulse)
  status_led.update();

  // Let the MQTT client do some work
  mqtt_client.loop();

  elapsed = millis() - now;
  if (elapsed > PERIOD) {
    PRINT("==== warning, loop overrun: ");
    PRINTLN(elapsed);
    mqtt_client.publish_timed_msg(now, TOPIC_MESSAGE, "LOOP OVERRUN");
    sleep_ms = 0;
  } else {
    sleep_ms = PERIOD - elapsed;
  }
  // Loop is variable period, but we do not really care...
  delay(sleep_ms);
}

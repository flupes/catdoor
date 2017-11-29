#include "RTClib.h"
#include "accel.h"
#include "daytimes.h"
#include "ledctrl.h"
#include "m0_hf_pwm.h"
#include "mqtt_client.h"
#include "netcfg.h"
#include "proxim.h"
#include "solenoids.h"
#include "utils.h"
#include "watchdog_timer.h"

// For float to string formatting
#include <avr/dtostrf.h>

// Where to get the battery voltage
#define VBATPIN A7

// Customization for acceptable daylight times
static const TimeSpan margin_after_sunrise(0, 0, 45, 0);
static const TimeSpan margin_before_sunset(0, 2, 0, 0);

static const unsigned long LOOP_PERIOD_MS = 10;

// To debug during dark period
static const bool debug_mode = false;

// global objects...
RTC_DS3231 rtc;
Accel accel_sensor;
Proxim proxim_sensor;
Solenoids& sol_actuators = Solenoids::Instance();
LedCtrl& status_led = LedCtrl::Instance();
WiFiClient wifi_client;
MQTT_Client mqtt_client(wifi_client, status_led);

// interupts function prototypes
void proximThreshold() { proxim_sensor.new_state = true; }

void accelReady() { accel_sensor.data_ready_ = true; }

void setup() {
  wdt_configure(11);

  // HF PWM
  pwm_configure();

  // let things stabilize before pulling too much current
  // not necessary, but surprise the developer less also
  delay(1000);

  // Force unjam, no matter what, just in case we rebooted
  // Also this says Hi to the world
  sol_actuators.unjam();

  // start in unkown state
  status_led.warning();

#ifdef USE_SERIAL
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect to start the app
  }
// Serial.print("sizeof(int)=");
// Serial.println(sizeof(int));
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
  accel_sensor.setDataRate(LIS3DH_DATARATE_25_HZ);
  accel_sensor.setRange(LIS3DH_RANGE_2_G);

  // attach hardware interrupt
  pinMode(A2, INPUT_PULLUP);
  attachInterrupt(A2, accelReady, RISING);
  delay(1);             // for some reason, this delay is CRITICAL!
  accel_sensor.read();  // read reset the signal --> start interrupts
  delay(1);
  accel_sensor.calibrate();

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

  // Configure pins for Adafruit ATWINC1500 Feather
  WiFi.setPins(8, 7, 4, 2);
  // Check for the presence of the shield
  PRINTLN("Connecting to the Wifi module...");
  if (WiFi.status() == WL_NO_SHIELD) {
    PRINTLN("WiFi shield not present");
    return;  // don't continue
  }

  mqtt_client.connect_wifi(MY_WIFI_SSID, MY_WIFI_PASS);
  status_led.warning();  // not done yet with setup!
  mqtt_client.setServer(MQTT_SERVER, MQTT_PORT);
  mqtt_client.connect_client();
  status_led.warning();  // not done yet with setup!

  PRINTLN("MQTT client connected.");
  const char* msg = "Catdoor v2 started";
  DateTime utc_time = rtc.now();
  unsigned int now = millis();
  DateTime local_time = utc_time.getLocalTime(TIME_ZONE_OFFSET);
  mqtt_client.sync_time(local_time, now);
  mqtt_client.publish_timed_msg(now, TOPIC_MESSAGE, msg);
  // start by flashing like in the darkness..
  status_led.alive();

  wdt_configure(4);
}

void loop() {
  static bool cat_exiting = false;
  static bool daylight = false;
  static bool rtc_read = true;
  static unsigned long last_time = millis();
  static unsigned long last_unjam = last_time;
  static unsigned long door_clear_time = last_time;
  unsigned long now = millis();
  unsigned long elapsed;
  unsigned long sleep_ms;
  static Solenoids::state_t prev_actuators_state = Solenoids::OFF;
  static char buffer[24];

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
      if (false || (morning < ltime && ltime < afternoon)) {
        status_led.ok();  // safe to repeat without breaking the pattern
        if (!daylight) {
          daylight = true;
          sprintf(buffer, "%s %d:%d", "UNLOCKED", afternoon.hour(),
                  afternoon.minute());
          mqtt_client.publish_timed_msg(now, TOPIC_MESSAGE, buffer);
        }
      }  // if daylight
      else {
        status_led.alive();  // safe to repeat without breaking the pattern
        if (daylight) {
          daylight = false;
          sprintf(buffer, "%s %d:%d", "LOCKED", morning.hour(),
                  morning.minute());
          mqtt_client.publish_timed_msg(now, TOPIC_MESSAGE, buffer);
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
      char num[6];
      dtostrf(measuredvbat, 5, 2, num);
      // char str[12];
      // sprintf(str, "VBAT %s", num);
      mqtt_client.publish_timed_msg(now, TOPIC_BATTERY_V, num);
      rtc_read = true;
    }

    last_time = now;
  }  // if 10s

  // Process accelerometer
  if (accel_sensor.data_ready_) {
    accel_sensor.process();
    if (accel_sensor.new_state_) {
      accel_sensor.new_state_ = false;
      mqtt_client.publish_timed_msg(
          now, TOPIC_DOORSTATE,
          ACCEL_STATES_NAMES[(uint8_t)accel_sensor.state_]);
    }
  }

  // Check if cat is in front of door
  if (proxim_sensor.new_state) {
    proxim_sensor.process();
    if (proxim_sensor.state == Proxim::CAT) {
      PRINTLN("CAT");
      mqtt_client.publish_timed_msg(now, TOPIC_PROXIMITY, "CAT");
      if ((daylight || debug_mode) && accel_sensor.state_ == Accel::CLOSED) {
        // Retract the door locks!
        if (sol_actuators.state() == Solenoids::OFF) {
          sol_actuators.open();
          mqtt_client.publish_timed_msg(now, TOPIC_SOLENOIDS, "RETRACTED");
        }
      }
    } else {
      PRINTLN("CLEAR");
      door_clear_time = now;
      mqtt_client.publish_timed_msg(now, TOPIC_PROXIMITY, "CLEAR");
    }
  }

  // Relase if the cat give up after 12s (to avoid a HOT_RELEASE much later)
  if (proxim_sensor.state == Proxim::CLEAR &&
      sol_actuators.state() == Solenoids::ON &&
      accel_sensor.state_ == Accel::CLOSED && (now - door_clear_time) > 12000) {
    sol_actuators.release();
    mqtt_client.publish_timed_msg(now, TOPIC_SOLENOIDS, "RELEASE");
  }

  // Mark that cat is going out (to release the solenoids later)
  if (sol_actuators.state() == Solenoids::ON &&
      accel_sensor.state_ == Accel::OPEN_OUT) {
    cat_exiting = true;
  }

  // Release solenoids if cat finished exiting
  if (cat_exiting && accel_sensor.state_ == Accel::CLOSED &&
      sol_actuators.state() == Solenoids::ON &&
      proxim_sensor.state == Proxim::CLEAR) {
    sol_actuators.release();
    mqtt_client.publish_timed_msg(now, TOPIC_SOLENOIDS, "RELEASE");
    cat_exiting = false;
  }

  // Detect jammed condition and try to resolve it
  if (accel_sensor.state_ == Accel::AJAR_OUT &&
      sol_actuators.state() == Solenoids::OFF) {
    if ((now - last_unjam) > Solenoids::COOLDOWN_DURATION_MS) {
      last_unjam = now;
      sol_actuators.unjam();
      mqtt_client.publish_timed_msg(now, TOPIC_SOLENOIDS, "UNJAM");
    }
  }

  // Check a hot release
  if (sol_actuators.state() != prev_actuators_state) {
    if (prev_actuators_state == Solenoids::ON &&
        sol_actuators.state() == Solenoids::HOT) {
      mqtt_client.publish_timed_msg(now, TOPIC_SOLENOIDS, "HOT_RELEASE");
    }
    prev_actuators_state = sol_actuators.state();
  }

  // Let the MQTT client do some work
  mqtt_client.loop();

  elapsed = millis() - now;
  if (elapsed > LOOP_PERIOD_MS) {
    PRINT("==== warning, loop overrun: ");
    PRINTLN(elapsed);
    mqtt_client.publish_timed_msg(now, TOPIC_MESSAGE, "LOOP OVERRUN");
    sleep_ms = 0;
  } else {
    sleep_ms = LOOP_PERIOD_MS - elapsed;
  }

  // Say we are still alive!
  wdt_reset();

  // Loop is variable period, but we do not really care...
  delay(sleep_ms);
}

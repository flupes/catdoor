#include <Arduino.h>

#include <RTCZero.h>

#include "basic_ntp.h"
#include "m0_hw_servo.h"
#include "netcfg.h"
#include "utils.h"
#include "watchdog_timer.h"

#define MAX_JAMMED_COUNT 5
#define UPDATE_MOT_PERIOD_MS 1000 * 2
#define UPDATE_NET_PERIOD_MS 1000 * 20

const uint8_t MORNING_HOUR = 8;
const uint8_t EVENING_HOUR = 17;

const uint32_t kClosedVoltageCount = 800;
const uint32_t kOpenVoltageCount = 2900;

M0HwServo servo(M0HwServo::D5, A1);

long g_lastReconnectAttempt = 0;
long g_lastMasterUpdate = 0;

enum LockState { CLOSED = 0, OPEN = 1, JAMMED = 2 };
static const char* STATE_NAMES[] = {"CLOSED", "OPEN", "JAMMED"};

LockState g_desiredDoorState = CLOSED;
bool g_autoLockMode = false;

AdafruitIO_Feed* command = g_io.feed("door-cmd");
AdafruitIO_Feed* state = g_io.feed("door-state");
AdafruitIO_Feed* battery = g_io.feed("door-battery");

RTCZero g_rtc;

void print_time() {
  PRINT("RTC Time = ");
  uint8_t hh = g_rtc.getHours();
  uint8_t mm = g_rtc.getMinutes();
  uint8_t ss = g_rtc.getSeconds();
  if (hh < 10) PRINT("0");
  PRINT(hh);
  PRINT(":");
  if (mm < 10) PRINT("0");
  PRINT(mm);
  PRINT(":");
  if (ss < 10) PRINT("0");
  PRINTLN(ss);
}

void handle_message(AdafruitIO_Data* data) {
  PRINT("received <- ");
  int value = data->toInt();
  PRINTLN(value);
  if (value == 2) {
    PRINTLN("automatic mode");
    g_autoLockMode = true;
  } else {
    g_autoLockMode = false;
    g_desiredDoorState = CLOSED;
    if (value == 1) {
      g_desiredDoorState = OPEN;
      PRINTLN("commanded to open");
    }
  }
}

void setup() {
#ifdef USE_SERIAL
  Serial.begin(115200);
  int wait_sec = 0;
  while (!Serial) {
    delay(1000);
    wait_sec++;
    if (wait_sec > 12) break;  // wait no more than 12s for a connection...
  }
#endif

  servo.Configure(450, 0, 2050, 120);
  // We always start in a closed state!
  // for our setup closed is MIN position, but be careful
  // to adjust is setup changes!
  servo.SetDegreeAngle(servo.GetMinDegreeAngle());

  g_rtc.begin();

  wdt_configure(11);

  // connect to g_io.adafruit.com
  PRINT("Connecting to Adafruit IO");
  g_io.connect();

  PRINTLN(g_io.connectionType());
  PRINTLN(g_io.networkStatus());

  command->onMessage(handle_message);

  // wait for a connection
  while (g_io.status() < AIO_CONNECTED) {
    PRINT(".");
    // PRINTLN(g_io.statusText());
    delay(1000);
  }

  // we are connected
  PRINTLN();
  PRINTLN(g_io.statusText());

  PRINT("Get time sync");
  time_t ts = 0;
  uint8_t nb_tries = 0;
  while (ts == 0 && nb_tries < 5) {
    PRINT(".");
    ts = get_local_ntp_time();
    nb_tries++;
  }

  if (ts == 0) {
    PRINTLN("Failed to get a time sync!");
  } else {
    g_rtc.setEpoch(ts);
    print_time();
  }

  // Force update
  command->get();

  WiFi.lowPowerMode();

  g_lastMasterUpdate = millis();
}

void loop() {
  static const int closed_deg = servo.GetMinDegreeAngle();
  static const int open_deg = servo.GetMaxDegreeAngle();

  static long elapsed_mot = millis();
  static long elapsed_net = millis();
  static int jammed_counter = MAX_JAMMED_COUNT;
  static LockState lock_state = JAMMED;

  long now = millis();

  // if (now - g_lastMasterUpdate > 45000) {
  //   // close if no update from master
  //   g_desiredDoorState = CLOSED;
  // }

  g_io.run();

  if (now - elapsed_mot > UPDATE_MOT_PERIOD_MS) {
    elapsed_mot = now;
    uint32_t current_voltage_count = servo.GetAveragedAnalogFeedback(3);

    jammed_counter--;
    if (current_voltage_count < kClosedVoltageCount) {
      lock_state = CLOSED;
      jammed_counter = MAX_JAMMED_COUNT;
    }
    if (current_voltage_count > kOpenVoltageCount) {
      lock_state = OPEN;
      jammed_counter = MAX_JAMMED_COUNT;
    }
    if (jammed_counter < 1) {
      lock_state = JAMMED;
      PRINTLN("JAMMED!!!");
    }

    if (g_autoLockMode) {
      uint8_t hh = g_rtc.getHours();
      g_desiredDoorState = CLOSED;
      if (MORNING_HOUR <= hh && hh <= EVENING_HOUR) {
        g_desiredDoorState = OPEN;
      }
    }
    if (g_desiredDoorState != lock_state) {
      if (g_desiredDoorState == OPEN) {
        servo.SetDegreeAngle(open_deg);
      } else {
        servo.SetDegreeAngle(closed_deg);
      }
    }
  }

  if (now - elapsed_net > UPDATE_NET_PERIOD_MS) {
    elapsed_net = now;
    PRINT("door state = ");
    PRINTLN(STATE_NAMES[lock_state]);
    state->save(lock_state);

    float measuredvbat = analogRead(A7);
    measuredvbat *= 2;     // we divided by 2, so multiply back
    measuredvbat *= 3.3;   // Multiply by 3.3V, our reference voltage
    measuredvbat /= 4096;  // convert to voltage (12 bits)
    PRINTLN("VBat: ");
    PRINTLN(measuredvbat);
    battery->save(measuredvbat);
  }

  wdt_reset();
  delay(200);
}

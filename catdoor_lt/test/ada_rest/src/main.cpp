#include <Arduino.h>

#include <time.h>

#include <RTCZero.h>

#include "basic_ntp.h"
#include "config.h"

#define LED_PIN 13

AdafruitIO_Feed *command = g_io.feed("door-cmd");
AdafruitIO_Feed *state = g_io.feed("door-state");

RTCZero g_rtc;

void handle_message(AdafruitIO_Data *data) {
  Serial.print("received <- ");
  int value = data->toInt();
  Serial.println(value);
  if (value == 1) {
    digitalWrite(LED_PIN, 1);
  } else {
    digitalWrite(LED_PIN, 0);
  }
}

void print_time() {
  Serial.print("RTC Time = ");
  uint8_t hh = g_rtc.getHours();
  uint8_t mm = g_rtc.getMinutes();
  uint8_t ss = g_rtc.getSeconds();
  if (hh < 10) Serial.print("0");
  Serial.print(hh);
  Serial.print(":");
  if (mm < 10) Serial.print("0");
  Serial.print(mm);
  Serial.print(":");
  if (ss < 10) Serial.print("0");
  Serial.println(ss);
}

void alarm_match() {
  Serial.println("alarm wake up!");
  print_time();
}

void setup() {
  pinMode(LED_PIN, OUTPUT);

  g_rtc.begin();

  Serial.begin(115200);
  int wait_sec = 0;
  while (!Serial) {
    delay(1000);
    wait_sec++;
    if (wait_sec > 12) break;
  }

  // connect to g_io.adafruit.com
  Serial.print("Connecting to Adafruit IO");
  g_io.connect();

  Serial.println(g_io.connectionType());
  Serial.println(g_io.networkStatus());

  command->onMessage(handle_message);

  // wait for a connection
  while (g_io.status() < AIO_CONNECTED) {
    Serial.print(".");
    // Serial.println(g_io.statusText());
    delay(1000);
  }

  // we are connected
  Serial.println();
  Serial.println(g_io.statusText());

  Serial.print("Get time sync");
  time_t ts = 0;
  uint8_t nb_tries = 0;
  while (ts == 0 && nb_tries < 5) {
    Serial.print(".");
    ts = get_local_ntp_time();
    nb_tries++;
  }

  if (ts == 0) {
    Serial.println("Failed to get a time sync!");
  } else {
    g_rtc.setEpoch(ts);
    print_time();
  }

  // Force update
  command->get();

  WiFi.lowPowerMode();

  // g_rtc.setAlarmSeconds(55);
  // g_rtc.enableAlarm(g_rtc.MATCH_SS);
  // g_rtc.attachInterrupt(alarm_match);
}

void loop() {
  static long last = millis();
  static bool flip = false;

  g_io.run();

  // Serial.println("main loop work...");
  long now = millis();
  if (now - last > 10000) {
    last = now;
    if (flip) {
      flip = false;
      state->save(0);
      Serial.println("send -> 0");
    } else {
      flip = true;
      state->save(1);
      Serial.println("send -> 1");
    }
  }
  delay(1000);

  // Serial.println("go to sleep for 1 min...");
  // g_rtc.standbyMode();
}

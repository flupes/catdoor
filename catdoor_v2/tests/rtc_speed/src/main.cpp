#define USE_SERIAL 1
#include "RTClib.h"
#include "utils.h"

RTC_DS3231 rtc;
char buffer[64];

void setup() {
  Serial.begin(115200);
  while (!Serial) {
    ;  // wait for serial port to connect. Needed for native USB port only
  }
  Serial.println("Start RTC speed test...");
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC!");
    while (1)
      ;
  }

  DateTime dt1(2017, 1, 1);
  dt1.toString(buffer);
  Serial.print("yDay(");
  Serial.print(buffer);
  Serial.print(") = ");
  Serial.println(dt1.yDay());

  DateTime dt2(2017, 3, 1);
  dt2.toString(buffer);
  Serial.print("yDay(");
  Serial.print(buffer);
  Serial.print(") = ");
  Serial.println(dt2.yDay());

  DateTime dt3(2020, 3, 1);
  dt3.toString(buffer);
  Serial.print("yDay(");
  Serial.print(buffer);
  Serial.print(") = ");
  Serial.println(dt3.yDay());

  DateTime dt7(2020, 12, 31);
  dt7.toString(buffer);
  Serial.print("yDay(");
  Serial.print(buffer);
  Serial.print(") = ");
  Serial.println(dt7.yDay());

  DateTime dt4(2017, 11, 11, 19, 0, 0, 0);
  dt4.toString(buffer);
  Serial.println("test conversions:");
  Serial.println(buffer);

  DateTime dt5 = dt4.getLocalTime(-8);
  dt5.toString(buffer);
  Serial.println(buffer);
  DateTime dt6 = dt4.getLocalTime(6);
  dt6.toString(buffer);
  Serial.println(buffer);
}

static const unsigned long DELAY = 200;

Timing rtc_comm("RTC_C", 10);
Timing rtc_timing_1("RTC_1", 10);
Timing rtc_timing_2("RTC_2", 10);

void loop() {
  static int counter = 0;

  // Serial.println(counter);
  rtc_comm.start();
  DateTime dt = rtc.now();
  rtc_comm.stop();

  if (counter % 10 == 0) {
    dt.toString(buffer);
    Serial.print("UTC = ");
    Serial.println(buffer);
  }
  rtc_timing_1.start();
  DateTime local = dt.getLocalTime(-8);
  rtc_timing_1.stop();

  rtc_timing_2.start();
  TimeSpan ts(0, 0, 8, 0);
  DateTime d2 = dt - ts;
  if (dt < d2) {
    Serial.println("true");
  }
  rtc_timing_2.stop();

  if (counter % 10 == 0) {
    local.toString(buffer);
    Serial.print("local = ");
    Serial.println(buffer);
  }

  delay(DELAY);
  counter++;
}

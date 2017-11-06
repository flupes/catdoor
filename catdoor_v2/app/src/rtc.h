#ifndef MY_RTC_H
#define MY_RTC_H

#include "RTClib.h"

class HourMinute {
 public:
  uint8_t hour;
  uint8_t minute;
  HourMinute() : hour(0), minute(0) {}
  HourMinute(const HourMinute& hm) {
    hour = hm.hour;
    minute = hm.minute;
  }
  HourMinute(uint8_t h, uint8_t m) : hour(h), minute(m) {}

  bool operator==(const HourMinute& hm) const {
    if (this->hour == hm.hour && this->minute == hm.minute) return true;
    return false;
  }

  bool operator<(const HourMinute& hm) const {
    if (this->hour > hm.hour) return false;
    if (this->hour < hm.hour) return true;
    if (this->minute < hm.minute) return true;
    return false;
  }

  bool operator>(const HourMinute& hm) const { return (hm < *this); }
};

class CatTime : public RTC_DS3231 {
 public:
  const static int8_t TIMEZONE = -8;
  HourMinute localTime() {
    DateTime gmt = now();
    uint8_t h, m;
    m = gmt.minute();
    if ((TIMEZONE + (int8_t)gmt.hour()) < 0) {
      h = TIMEZONE + 24 + (int8_t)gmt.hour();
    } else {
      if ((TIMEZONE + (int8_t)gmt.hour()) > 23) {
        h = TIMEZONE + gmt.hour() - 24;
      } else {
        h = TIMEZONE + gmt.hour();
      }
    }
    return HourMinute(h, m);
  }
};

#endif

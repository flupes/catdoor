#ifndef _CATDOOR_LEDCTRL_H_
#define _CATDOOR_LEDCTRL_H_

#include <stdint.h>
#include "VisualFeedback.h"

class LedCtrl : public VisualFeedback {
 public:
  static LedCtrl& Instance() {
    static LedCtrl instance_;
    return instance_;
  }

  LedCtrl(LedCtrl const&) = delete;
  void operator=(LedCtrl const&) = delete;

  typedef enum { MANUAL, PULSATING, FLASHING } mode_t;

  void on();

  void off();

  void pulse(uint16_t period_ms);

  void flash(uint16_t on_time_ms, uint16_t off_time_ms);

  void update();

  void error() { flash(100, 100); }

  void warning() { flash(200, 400); }

  void ok() { pulse(6000); }

  void alive() { flash(80, 6000); }

  static const uint8_t PIN_L = 11;

 private:
  LedCtrl() : mode_(MANUAL) {}
  mode_t mode_;
  uint16_t period_;
  uint16_t ontime_;
  uint16_t offtime_;
  unsigned long last_;
};

#endif

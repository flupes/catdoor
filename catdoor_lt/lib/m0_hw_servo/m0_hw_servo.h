/*
  Hardware based PWM servo control on a SAMD21 (Cortex M0)

  Rather than the software Arduino PWM implementation, use the M0 TCC system
  to generate a PWM signal taking advantage of the M0 capabilities.
  Because of the very customized clock + signal muxing, this code only
  controls a PWM signal on 2 pins:
    - D5
    - D6
*/

#ifndef _M0_HW_SERVO_H
#define _M0_HW_SERVO_H_

#include <stdint.h>

class M0HwServo {

public:
  enum ServoPin {UNDEF=0, D5=5, D6=6};

  M0HwServo(ServoPin pwmPin, uint32_t analogPin = 0);

  void Configure(uint32_t minPeriod, int minDegree, uint32_t maxPeriod, int maxDegree);

  bool SetPeriod(uint32_t msPeriod);

  bool SetDegreeAngle(int degAngle);

  uint32_t GetAnalogFeedback();

  uint32_t GetAveragedAnalogFeedback(uint8_t nbSamples, uint32_t periodMs = 10);

  uint32_t GetMinPeriod() {
    return min_period_;
  }

  uint32_t GetMaxPeriod() {
    return max_period_;
  }

  int GetMinDegreeAngle() {
    return min_angle_;
  }

  int GetMaxDegreeAngle() {
    return max_angle_;
  }

  uint32_t GetCurrentMsPeriod() {
    return current_period_;
  }

private:
  uint32_t min_period_, max_period_, current_period_;
  int min_angle_, max_angle_, current_angle_;

  uint32_t pwm_pin_;
  uint32_t analog_pin_;

  M0HwServo();
  static bool initialized_;

  // Configure the TTC system for PWM signal generation
  static void pwm_configure();
  // Configure the period duration on the given pin (5 or 6 only)
  static int pwm_set(uint8_t pin, uint32_t us);

};


#endif

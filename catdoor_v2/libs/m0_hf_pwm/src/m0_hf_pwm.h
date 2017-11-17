/*
  High Frequency PWM generation and control on a SAMD21 (Cortex M0)

  Rather than the slow Arduino PWM implementation, use the M0 TCC system
  to generate a high frequency PWM signal.
  Because of the very customize clock + signal muxing, this code only
  controls PWM on 3 pins:
    - D5
    - D6
    - D11
*/

#ifndef _M0_HF_PWM_H_
#define _M0_HF_PWM_H_

#include <stdint.h>

// Initialize the HF PWM system
extern void pwm_configure();

// Return the max count of PWM (=signal high)
extern uint32_t pwm_get_max();

// Configure the PWM width on the given pin (5, 6 or 11)
extern int pwm_set(uint8_t pin, uint32_t width);

#endif

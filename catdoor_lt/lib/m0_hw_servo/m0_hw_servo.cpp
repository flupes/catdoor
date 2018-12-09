// Configure three pins (D5 and D6) on a TCC timer on the Adafruit Feather
// M0 (SAMD21) for fast HW PWM signal.

// Inspired from: MartinL @ https://forum.arduino.cc/index.php?topic=346731.0
// Essentially post #2, #6, and #15
// Pinout for the feather:
// https://cdn-learn.adafruit.com/assets/assets/000/041/593/original/Adafruit_Feather_M0_WiFi_v1_0.pdf?1494356530
// Atmel SAMD21 documentation:
// http://ww1.microchip.com/downloads/en/DeviceDoc/40001882A.pdf

#include "m0_hw_servo.h"
#include <Arduino.h>

static const unsigned char g_gen_clock_pwm = 4;

bool M0HwServo::initialized_ = false;

M0HwServo::M0HwServo(ServoPin pwmPin, uint32_t analogPin)
    : pwm_pin_(UNDEF), analog_pin_(analogPin) {
  if (!initialized_) {
    M0HwServo::pwm_configure();
  }
  min_period_ = 1000;
  max_period_ = 2000;
  min_angle_ = 0;
  max_angle_ = 90;
  if (pwmPin == D5 || pwmPin == D6) {
    pwm_pin_ = pwmPin;
    if (analog_pin_ != 0) {
      // pinMode(analog_pin_, INPUT);
    }
  }
}

uint32_t M0HwServo::GetAnalogFeedback() {
  if (analog_pin_ != 0) {
    return analogRead(analog_pin_);
  }
  return 0;
}

uint32_t M0HwServo::GetAveragedAnalogFeedback(uint8_t nbSamples,
                                              uint32_t periodMs) {
  uint32_t voltage = 0.0;
  for (uint8_t i = 0; i < nbSamples; i++) {
    voltage += GetAnalogFeedback();
    delay(periodMs);
  }
  return voltage / nbSamples;
}

void M0HwServo::Configure(uint32_t minPeriod, int minDegree, uint32_t maxPeriod,
                          int maxDegree) {
  min_period_ = minPeriod;
  max_period_ = maxPeriod;
  min_angle_ = minDegree;
  max_angle_ = maxDegree;
}

bool M0HwServo::SetPeriod(uint32_t msPeriod) {
  if (pwm_pin_ != 0) {
    if (msPeriod >= min_period_ && msPeriod <= max_period_) {
      M0HwServo::pwm_set(pwm_pin_, msPeriod);
      current_period_ = msPeriod;
      return true;
    }
  }
  return false;
}

bool M0HwServo::SetDegreeAngle(int degAngle) {
  if (pwm_pin_ != 0) {
    if (degAngle >= min_angle_ && degAngle <= max_angle_) {
      current_angle_ = degAngle;
      return SetPeriod(min_period_ + degAngle * ((max_period_ - min_period_) /
                                                 (max_angle_ - min_angle_)));
    }
  }
  return false;
}

void M0HwServo::pwm_configure() {
  REG_GCLK_GENDIV =
      GCLK_GENDIV_DIV(
          3) |  // Divide the 48MHz clock source by divisor 3: 48MHz/3=16MHz
      GCLK_GENDIV_ID(g_gen_clock_pwm);  // Select Generic Clock (GCLK) 4
  while (GCLK->STATUS.bit.SYNCBUSY)
    ;  // Wait for synchronization

  REG_GCLK_GENCTRL = GCLK_GENCTRL_IDC |  // Set the duty cycle to 50/50 HIGH/LOW
                     GCLK_GENCTRL_GENEN |        // Enable GCLK4
                     GCLK_GENCTRL_SRC_DFLL48M |  // Set the 48MHz clock source
                     GCLK_GENCTRL_ID(g_gen_clock_pwm);  // Select GCLK4
  while (GCLK->STATUS.bit.SYNCBUSY)
    ;  // Wait for synchronization

  // Enable the port multiplexer for digital pon D5(PA15), D6(PA20)
  PORT->Group[g_APinDescription[5].ulPort]
      .PINCFG[g_APinDescription[5].ulPin]
      .bit.PMUXEN = 1;
  PORT->Group[g_APinDescription[6].ulPort]
      .PINCFG[g_APinDescription[6].ulPin]
      .bit.PMUXEN = 1;

  // Connect TCC0 timer to PA15=D5 (ODD pin,  Function F --> TTC0/WO[5] = CCB1)
  // Why we access PA15 from D2 is still totally magic too me.
  // It would be so much easier to reference the actual SAMD21 port/pin
  // directly...
  PORT->Group[g_APinDescription[2].ulPort]
      .PMUX[g_APinDescription[2].ulPin >> 1]
      .reg = PORT_PMUX_PMUXO_F;
  while (GCLK->STATUS.bit.SYNCBUSY)
    ;  // Wait for synchronization

  // Connect TCC0 timer to PA20=D6 (EVEN pin,  Function F --> TTC0/WO[6] = CCB2)
  PORT->Group[g_APinDescription[6].ulPort]
      .PMUX[g_APinDescription[6].ulPin >> 1]
      .reg = PORT_PMUX_PMUXE_F;
  while (GCLK->STATUS.bit.SYNCBUSY)
    ;  // Wait for synchronization

  // Feed GCLK4 to TCC0 (and TCC1)
  REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN |      // Enable GCLK4 to TCC0 and TCC1
                     GCLK_CLKCTRL_GEN_GCLK4 |  // Select GCLK4
                     GCLK_CLKCTRL_ID_TCC0_TCC1;  // Feed GCLK4 to TCC0 and TCC1
  while (GCLK->STATUS.bit.SYNCBUSY)
    ;  // Wait for synchronization

  // Dual slope PWM operation: timers countinuously count up to PER register
  // value then down 0
  REG_TCC0_WAVE |=
      TCC_WAVE_POL(0xF) |  // Reverse the output polarity on all TCC0 outputs
      TCC_WAVE_WAVEGEN_DSBOTTOM;  // Setup dual slope PWM on TCC0
  while (TCC0->SYNCBUSY.bit.WAVE)
    ;  // Wait for synchronization

  // Each timer counts up to a maximum or TOP value set by the PER register,
  // this determines the frequency of the PWM operation:
  // 20000 = 50Hz, 10000 = 100Hz, 2500  = 400Hz
  REG_TCC0_PER = 20000;  // Set the frequency of the PWM on TCC0 to 50Hz
  while (TCC0->SYNCBUSY.bit.PER)
    ;

  // The CCBx register value corresponds to the pulsewidth in microseconds (us)
  REG_TCC0_CCB1 = 1500;  // TCC0 CCB1 - center the servo on D5
  while (TCC0->SYNCBUSY.bit.CCB1)
    ;
  REG_TCC0_CCB2 = 1500;  // TCC0 CCB2 - center the servo on D6
  while (TCC0->SYNCBUSY.bit.CCB2)
    ;

  // Divide the 16MHz signal by 8 giving 2MHz (0.5us) TCC0 timer tick and enable
  // the outputs
  REG_TCC0_CTRLA |= TCC_CTRLA_PRESCALER_DIV8 |  // Divide GCLK4 by 8
                    TCC_CTRLA_ENABLE;           // Enable the TCC0 output
  while (TCC0->SYNCBUSY.bit.ENABLE)
    ;  // Wait for synchronization

  // not belong here, but what the heck...
  analogReadResolution(12);

  initialized_ = true;
}

int M0HwServo::pwm_set(uint8_t pin, uint32_t us) {
  if (pin == 5) {
    REG_TCC0_CCB1 = us;
    while (TCC0->SYNCBUSY.bit.CCB1)
      ;
  } else if (pin == 6) {
    REG_TCC0_CCB2 = us;
    while (TCC0->SYNCBUSY.bit.CCB2)
      ;
  } else {
    return 0;
  }
  return 1;
}

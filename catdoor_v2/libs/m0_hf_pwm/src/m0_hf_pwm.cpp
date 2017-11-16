// Configure three pins (D5, D6 and D11) on a TCC timer on the Adafruit Feather
// M0 (SAMD21) for fast PWM (higher frequency than a cat can perceive > 73KHz,
// otherwise the solenoid is singing and the catdoor assembly resonate very
// loudly;-)

// Inspired from: MartinL @ https://forum.arduino.cc/index.php?topic=346731.0
// Essentially post #2, #6, #107  and #109
// Pinout for the feather:
// https://cdn-learn.adafruit.com/assets/assets/000/041/593/original/Adafruit_Feather_M0_WiFi_v1_0.pdf?1494356530
// Atmel SAMD21 documentation:
// http://ww1.microchip.com/downloads/en/DeviceDoc/40001882A.pdf

#include "Arduino.h"

static const unsigned char g_gen_clock_pwm = 4;
static const uint32_t g_max_per = 512;

uint32_t pwm_get_max() { return g_max_per; }

void pwm_configure() {
  REG_GCLK_GENDIV =
      GCLK_GENDIV_DIV(
          1) |  // Divide the 48MHz clock source by divisor 1: 48MHz/4=12MHz
      GCLK_GENDIV_ID(g_gen_clock_pwm);  // Select Generic Clock (GCLK) 4
  while (GCLK->STATUS.bit.SYNCBUSY)
    ;  // Wait for synchronization

  REG_GCLK_GENCTRL = GCLK_GENCTRL_IDC |  // Set the duty cycle to 50/50 HIGH/LOW
                     GCLK_GENCTRL_GENEN |        // Enable GCLK4
                     GCLK_GENCTRL_SRC_DFLL48M |  // Set the 48MHz clock source
                     GCLK_GENCTRL_ID(g_gen_clock_pwm);  // Select GCLK4
  while (GCLK->STATUS.bit.SYNCBUSY)
    ;  // Wait for synchronization

  // Enable the port multiplexer for digital pon D5(PA15), D6(PA20) and
  // D11(PA16)
  PORT->Group[g_APinDescription[5].ulPort]
      .PINCFG[g_APinDescription[5].ulPin]
      .bit.PMUXEN = 1;
  PORT->Group[g_APinDescription[6].ulPort]
      .PINCFG[g_APinDescription[6].ulPin]
      .bit.PMUXEN = 1;
  PORT->Group[g_APinDescription[11].ulPort]
      .PINCFG[g_APinDescription[11].ulPin]
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

  // Connect TCC2 timer to PA16=D11 (EVEN pin, Function E --> TTC2/WO[0] = CCB0)
  PORT->Group[g_APinDescription[11].ulPort]
      .PMUX[g_APinDescription[11].ulPin >> 1]
      .reg = PORT_PMUX_PMUXE_E;
  while (GCLK->STATUS.bit.SYNCBUSY)
    ;  // Wait for synchronization

  // Feed GCLK4 to TCC0 (and TCC1)
  REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN |      // Enable GCLK4 to TCC0 and TCC1
                     GCLK_CLKCTRL_GEN_GCLK4 |  // Select GCLK4
                     GCLK_CLKCTRL_ID_TCC0_TCC1;  // Feed GCLK4 to TCC0 and TCC1
  while (GCLK->STATUS.bit.SYNCBUSY)
    ;  // Wait for synchronization

  // Feed GCLK4 to TCC2 (and TC3)
  REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN |      // Enable GCLK4 to TCC2 (and TC3)
                     GCLK_CLKCTRL_GEN_GCLK4 |  // Select GCLK4
                     GCLK_CLKCTRL_ID_TCC2_TC3;  // Feed GCLK4 to TCC2 (and TC3)
  while (GCLK->STATUS.bit.SYNCBUSY)
    ;  // Wait for synchronization

  // Normal (single slope) PWM operation: timers countinuously count up to PER
  // register value and then is reset to 0
  REG_TCC0_WAVE |= TCC_WAVE_WAVEGEN_NPWM;  // Setup single slope PWM on TCC1
  while (TCC0->SYNCBUSY.bit.WAVE)
    ;  // Wait for synchronization

  REG_TCC2_WAVE |= TCC_WAVE_WAVEGEN_NPWM;  // Setup single slope PWM on TCC1
  while (TCC2->SYNCBUSY.bit.WAVE)
    ;  // Wait for synchronization

  // Each timer counts up to a maximum or TOP value set by the PER register,
  // this determines the frequency of the PWM operation 12EHz / 512 = 93.75kHz
  REG_TCC0_PER = g_max_per;  // Set the frequency of the PWM on TCC0 to 93kHz >
                             // cat auditible range
  while (TCC0->SYNCBUSY.bit.PER)
    ;

  REG_TCC2_PER = g_max_per;  // Set the frequency of the PWM on TCC2 to 93kHz
  while (TCC2->SYNCBUSY.bit.PER)
    ;

  // It is possible to invert the wave of D5 controlled by TCC0/WO[5]
  // REG_TCC0_DRVCTRL |= TCC_DRVCTRL_INVEN5;

  // The CCBx register value corresponds to the pulsewidth. Default is set to
  // 50%

  REG_TCC0_CC1 = 0;
  // Set the duty cycle of the PWM on TCC0 W[2] to 50% (for D10)
  // REG_TCC0_CC1 = 256;
  while (TCC0->SYNCBUSY.bit.CC1)
    ;
  REG_TCC0_CC2 = 0;
  // Set the duty cycle of the PWM on TCC0 W[3] to 50% (for D12)
  // REG_TCC0_CC2 = 256;
  while (TCC0->SYNCBUSY.bit.CC2)
    ;

  REG_TCC2_CC0 = 0;
  // REG_TCC2_CC0 = 256;
  while (TCC2->SYNCBUSY.bit.CC0)
    ;

  // Enable TCC0 timer
  REG_TCC0_CTRLA |= TCC_CTRLA_PRESCALER_DIV1 |  // Divide GCLK4 by 1
                    TCC_CTRLA_ENABLE;           // Enable the TCC0 output
  while (TCC0->SYNCBUSY.bit.ENABLE)
    ;  // Wait for synchronization

  // Enable TCC2 timer
  REG_TCC2_CTRLA |= TCC_CTRLA_PRESCALER_DIV1 |  // Divide GCLK4 by 1
                    TCC_CTRLA_ENABLE;           // Enable the TCC0 output
  while (TCC2->SYNCBUSY.bit.ENABLE)
    ;  // Wait for synchronization
}

bool pwm_set(uint8_t pin, uint32_t width) {
  if (width > g_max_per) return false;
  if (pin == 5) {
    REG_TCC0_CC1 = width;
    while (TCC0->SYNCBUSY.bit.CC1)
      ;
  } else if (pin == 6) {
    REG_TCC0_CC2 = width;
    while (TCC0->SYNCBUSY.bit.CC2)
      ;
  } else if (pin == 11) {
    REG_TCC2_CC0 = width;
    while (TCC2->SYNCBUSY.bit.CC0)
      ;
  } else {
    return false;
  }
  return true;
}

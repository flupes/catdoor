// Configure two pin (D10 + D12) on a TCC timer on the Adafruit Feather M0 (SAMD21)
// for fast PWM (higher frequency than a cat can perceive > 73KHz, otherwise the 
// solenoid is singing and the catdoor assembly resonate very loudly;-)

// Inspired from: MartinL @ https://forum.arduino.cc/index.php?topic=346731.0
// Essentially post #2, #6, #107  and #109
// Pinout for the feather:
// https://cdn-learn.adafruit.com/assets/assets/000/041/593/original/Adafruit_Feather_M0_WiFi_v1_0.pdf?1494356530
// Atmel SAMD21 documentation:
// http://ww1.microchip.com/downloads/en/DeviceDoc/40001882A.pdf

static const unsigned char g_gen_clock = 4;
static const uint32_t g_max_per = 512;

void pwm_configure()
{
  REG_GCLK_GENDIV = GCLK_GENDIV_DIV(1) |           // Divide the 48MHz clock source by divisor 1: 48MHz/4=12MHz
                    GCLK_GENDIV_ID(g_gen_clock);        // Select Generic Clock (GCLK) 4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  REG_GCLK_GENCTRL = GCLK_GENCTRL_IDC |           // Set the duty cycle to 50/50 HIGH/LOW
                     GCLK_GENCTRL_GENEN |         // Enable GCLK4
                     GCLK_GENCTRL_SRC_DFLL48M |   // Set the 48MHz clock source
                     GCLK_GENCTRL_ID(g_gen_clock);     // Select GCLK4
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  // Enable the port multiplexer for digital pon D10(PA018) and D12(PA19)
  PORT->Group[g_APinDescription[10].ulPort].PINCFG[g_APinDescription[10].ulPin].bit.PMUXEN = 1;
  PORT->Group[g_APinDescription[12].ulPort].PINCFG[g_APinDescription[12].ulPin].bit.PMUXEN = 1;

  // Connect TCC0 timer to PA18 (PMUX_PMUXO = Odd) and PA19 (PMUX_PMUXE = Even), section F
  PORT->Group[g_APinDescription[10].ulPort].PMUX[g_APinDescription[10].ulPin >> 1].reg =
    PORT_PMUX_PMUXO_F | PORT_PMUX_PMUXE_F;
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  // Feed GCLK4 to TCC0 and TCC1
  REG_GCLK_CLKCTRL = GCLK_CLKCTRL_CLKEN |         // Enable GCLK4 to TCC0 and TCC1
                     GCLK_CLKCTRL_GEN_GCLK4 |     // Select GCLK4
                     GCLK_CLKCTRL_ID_TCC0_TCC1;   // Feed GCLK4 to TCC0 and TCC1
  while (GCLK->STATUS.bit.SYNCBUSY);              // Wait for synchronization

  // Normal (single slope) PWM operation: timers countinuously count up to PER register value and then is reset to 0
  REG_TCC0_WAVE |= TCC_WAVE_WAVEGEN_NPWM;        // Setup single slope PWM on TCC1
  while (TCC0->SYNCBUSY.bit.WAVE);               // Wait for synchronization

  // Each timer counts up to a maximum or TOP value set by the PER register,
  // this determines the frequency of the PWM operation 12EHz / 512 = 93.75kHz
  REG_TCC0_PER = g_max_per;      // Set the frequency of the PWM on TCC0 to 93kHz > cat auditible range
  while (TCC0->SYNCBUSY.bit.PER);

  // The CCBx register value corresponds to the pulsewidth. Default is set to 50%
  REG_TCC0_CC2 = 256;      // Set the duty cycle of the PWM on TCC0 W[2] to 50% (for D10)
  REG_TCC0_CC3 = 256;      // Set the duty cycle of the PWM on TCC0 W[3] to 50% (for D12)
  while (TCC0->SYNCBUSY.bit.CC1);

  // Enable TCC0 timer
  REG_TCC0_CTRLA |= TCC_CTRLA_PRESCALER_DIV1 |    // Divide GCLK4 by 1
                    TCC_CTRLA_ENABLE;             // Enable the TCC0 output
  while (TCC0->SYNCBUSY.bit.ENABLE);              // Wait for synchronization
}


bool pwm_set(uint8_t pin, uint32_t width)
{
  if (width > g_max_per) return false;
  if (pin == 10) {
    REG_TCC0_CC2 = width;
  }
  else if (pin == 12 ) {
    REG_TCC0_CC3 = width;
  }
  else {
    return false;
  }
  while (TCC0->SYNCBUSY.bit.CC1);
  return true;
}


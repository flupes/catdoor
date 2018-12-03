#include <sam.h>

#include "watchdog_timer.h"

static const unsigned char g_gen_clock_wdt = 5;

static inline void wdt_sync() {
  while (WDT->STATUS.bit.SYNCBUSY == 1)
    ;  // Just wait till WDT is free
}

void wdt_system_reset() {
  // use the WDT watchdog timer to force a system reset.
  // WDT MUST be running for this to work
  WDT->CLEAR.reg = 0x00;  // system reset via WDT
  wdt_sync();
}

void wdt_reset() {
  // reset the WDT watchdog timer.
  // this must be called before the WDT resets the system
  WDT->CLEAR.reg = 0xA5;  // reset the WDT
  wdt_sync();
}

void wdt_configure(uint8_t period) {
  // initialize the WDT watchdog timer
  // Do one-time initialization of the watchdog timer.

  // Setup GCLK for the watchdog using:
  // - Generic clock generator 2 as the source for the watchdog clock
  // - Low power 32khz internal oscillator as the source for generic clock
  //   generator 2.
  // - Generic clock generator 2 divisor to 32 so it ticks roughly once a
  //   millisecond.

  // Set generic clock generator 2 divisor to 4 so the clock divisor is 32.
  // From the datasheet the clock divisor is calculated as:
  //   2^(divisor register value + 1)
  // A 32khz clock with a divisor of 32 will then generate a 1ms clock period.
  // GCLK->GENDIV.reg = GCLK_GENDIV_ID(g_gen_clock_wdt) | GCLK_GENDIV_DIV(4);

  // A 32khz clock with a divisor of 64 will then generate a 1ms clock period.
  GCLK->GENDIV.reg = GCLK_GENDIV_ID(g_gen_clock_wdt) | GCLK_GENDIV_DIV(5);

  // Now enable clock generator 2 using the low power 32khz oscillator and the
  // clock divisor set above.
  GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(g_gen_clock_wdt) | GCLK_GENCTRL_GENEN |
                      GCLK_GENCTRL_SRC_OSCULP32K | GCLK_GENCTRL_DIVSEL;
  while (GCLK->STATUS.bit.SYNCBUSY)
    ;  // Syncronize write to GENCTRL reg.
  // Turn on the WDT clock using clock generator 2 as the source.
  GCLK->CLKCTRL.reg =
      GCLK_CLKCTRL_ID_WDT | GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK5;

  WDT->CTRL.reg = 0;  // disable watchdog
  wdt_sync();         // sync is required

  WDT->CONFIG.reg =
      min(period, 11);  // see Table 17-5 Timeout Period (valid values 0-11)

  WDT->CTRL.reg = WDT_CTRL_ENABLE;  // enable watchdog
  wdt_sync();
}

void wdt_disable() {
  WDT->CTRL.bit.ENABLE = 0;
  wdt_sync();
}

// From: https://forum.arduino.cc/index.php?topic=366836.0
// rickrlh & rbrucemtl

#include <Arduino.h>

// Configure the watchdog timer with a given period
// 11 -> 32s
// 10 -> 16s
// 9 -> 8s
// 8 -> 4s
// 7 -> 2s
// 6 -> 1s
// 5 -> 500ms
// 4 -> 250ms
// 3 -> 125ms
// 2 -> 62ms
// 1 -> 31ms
extern void wdt_configure(uint8_t period);

// Disable the WDT
extern void wdt_disable();

// Reset the watchdog timer (avoid system reset)
extern void wdt_reset();

// System reset: do not call ;-)
extern void wdt_system_reset();

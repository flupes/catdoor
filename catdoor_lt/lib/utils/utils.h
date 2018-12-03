#ifndef _CATDOOR_UTILS_H_
#define _CATDOOR_UTILS_H_

#include <stdio.h>  // for size_t

#define USE_SERIAL

#ifdef USE_SERIAL
#define PRINT(x) Serial.print(x)
#define PRINTLN(x) Serial.println(x)
#else
#define PRINT(x)
#define PRINTLN(x)
#endif

#endif

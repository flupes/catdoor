#ifndef CATDOOR_H
#define CATDOOR_H

#ifdef USE_SERIAL
#define PRINT(x) Serial.print(x)
#define PRINTLN(x) Serial.println(x)
#else
#define PRINT(x)
#define PRINTLN(x)
#endif

class Timing {
  uint16_t nsamples;
  uint16_t counter;
  unsigned long accumulation;
  unsigned long startus;
  char name[8];

 public:
  Timing(const char *timer_name, uint16_t number_of_samples)
      : nsamples(number_of_samples) {
    accumulation = 0;
    counter = 0;
    size_t len = strlen(timer_name);
    strncpy(name, timer_name, (len > 8) ? 8 : len);
  }
  void start() {
#ifdef USE_SERIAL
    startus = micros();
#endif
  }
  void stop() {
#ifdef USE_SERIAL
    accumulation += (micros() - startus);
    counter++;
    if (counter >= nsamples) {
      PRINT("Avergage (n=");
      PRINT(nsamples);
      PRINT(") for Timer [");
      PRINT(name);
      PRINT("] = ");
      PRINT(accumulation / nsamples);
      PRINTLN(" us");
      accumulation = 0;
      counter = 0;
    }
#endif
  }
};

#endif

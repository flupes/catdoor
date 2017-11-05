#ifndef CATDOOR_H
#define CATDOOR_H

#include "ADA_LIS3DH.h"
#include "ADA_VCNL4010.h"

#include "m0_hf_pwm.h"

#ifdef USE_SERIAL
#define PRINT(x) Serial.print(x)
#define PRINTLN(x) Serial.println(x)
#else
#define PRINT(x)
#define PRINTLN(x)
#endif

#ifdef USE_SERIAL
static const char *ACCEL_STATES_NAMES[] = {"CLOSED", "OPEN_IN", "OPEN_OUT",
                                           "JAMMED", "AJAR_IN"};
#endif

class Accel : public ADA_LIS3DH {
 public:
  static const int16_t V_CLOSED = -15900;
  static const int16_t H_CLOSED = 400;
  static const int16_t V_JAMMED = -15500;
  static const int16_t H_JAMMED = 3000;
  static const int16_t V_OPENED = -13000;
  static const int16_t H_OPENED = 10000;
  static const uint8_t JAMCOUNTS = 12;  // 1.2s at 10Hz
  // Horizontal:
  // Zaccel > 0 --> open IN
  // Zaccel < 0 --> open OUT
  typedef enum {
    CLOSED = 0,
    OPEN_IN = 1,
    OPEN_OUT = 2,
    JAMMED = 3,
    AJAR_IN = 4
  } state_t;
  state_t state;
  bool new_state;
  uint8_t jamcounter;
  Accel(void) : ADA_LIS3DH(), state(CLOSED), new_state(false), jamcounter(0) {}
  void callback() {
    // read get the lastest data and at the same time clears the interupt signal
    read();
    // PRINT(accel[0]);
    // PRINT("\t");
    // PRINT(accel[2]);
    // PRINTLN();
    // compute new state
    if (accel[0] < V_CLOSED && abs(accel[2]) < H_CLOSED) {
      if (state != CLOSED) {
        state = CLOSED;
        jamcounter = 0;
        new_state = true;
      }
      return;
    }
    if (accel[0] > V_OPENED) {
      if (accel[2] > H_OPENED) {
        if (state != OPEN_IN) {
          state = OPEN_IN;
          jamcounter = 0;
          new_state = true;
        }
        return;
      }
      if (accel[2] < -H_OPENED) {
        if (state != OPEN_OUT) {
          state = OPEN_OUT;
          jamcounter = 0;
          new_state = true;
        }
        return;
      }
    }
    if (accel[0] >= V_CLOSED && accel[2] >= H_CLOSED) {
      if (state != AJAR_IN) {
        state = AJAR_IN;
        jamcounter = 0;
        new_state = true;
      }
      return;
    }
    if (accel[0] < V_JAMMED && accel[2] < H_CLOSED && accel[2] > -H_JAMMED) {
      // only consider jamed state when door is out
      if (state != JAMMED) {
        jamcounter++;
        // Make sure we are totally jammed, and not trough a transition
        if (jamcounter > JAMCOUNTS) {
          state = JAMMED;
          new_state = true;
        }
      }
      return;
    }
    // PRINTLN("Undefined angle: no state change");
  }
  void process() {
    new_state = false;
#ifdef USE_SERIAL
    PRINTLN(ACCEL_STATES_NAMES[(uint8_t)state]);
#endif
  }
};

class Proxim : public ADA_VCNL4010 {
 public:
  static const uint16_t THRESHOLD = 2100;
  typedef enum { CLEAR, CAT } state_t;
  state_t state;
  bool new_state;
  uint8_t istatus;
  Proxim(void) : ADA_VCNL4010(), state(CLEAR), new_state(false) {}
  void callback() {
    istatus = readInterruptStatus();
    // PRINTLN(istatus);
    clearInterrupt(istatus);
    if (istatus == 1) {
      setLowThreshold(THRESHOLD);
      setHighThreshold(5000);
    } else {
      setLowThreshold(0);
      setHighThreshold(THRESHOLD);
    }
    activateProximityThresholdInterrupt();
    new_state = true;
  }
  void process() {
    if (istatus == 1) {
      state = CAT;
      PRINTLN("CAT");
    } else {
      state = CLEAR;
      PRINTLN("CLEAR");
    }
    new_state = false;
  }
};

class Solenoids {
 public:
  static const uint8_t PIN_A = 5;
  static const uint8_t PIN_B = 6;
  static const unsigned long MAX_ON_DURATION = 12000;
  static const unsigned long COOLDOWN_DURATION = 16000;

  static Solenoids *instance;
  typedef enum {
    OFF = 0,
    ON = 1,
    MAX_A = 2,
    STAY_A = 3,
    MAX_B = 4,
    HOT = 5
  } state_t;
  state_t state;
  unsigned long start_time;
  unsigned long stop_time;
  unsigned long factor;
  Solenoids() : state(OFF) {
    if (!instance) {
      // Initial High Frequency PWM setup
      pwm_configure();
      instance = this;
    }
    pwm_set(PIN_A, 0);
    pwm_set(PIN_B, 0);
  }

  void open(bool boost = false) {
    PRINT("Solenoids state = ");
    PRINTLN(state);
    if (boost) {
      factor = 4;
    } else {
      factor = 1;
    }
    if (state == OFF) {
      start_time = millis();
      state = MAX_A;
      pwm_set(PIN_A, 512);
    }
  }

  void release() {
    pwm_set(PIN_A, 0);
    pwm_set(PIN_B, 0);
    state = OFF;
  }

  void process() {
    unsigned long now = millis();
    if (state == MAX_A) {
      if ((now - start_time) > factor * 120) {
        pwm_set(PIN_A, 200);
        state = STAY_A;
      }
    }
    if (state == STAY_A) {
      if ((now - start_time) > factor * 150) {
        pwm_set(PIN_B, 512);
        state = MAX_B;
      }
    }
    if (state == MAX_B) {
      if ((now - start_time) > factor * 300) {
        pwm_set(PIN_B, 200);
        state = ON;
      }
    }
    if (state == ON) {
      if ((now - start_time) > MAX_ON_DURATION) {
        release();
        state = HOT;
        stop_time = now;
      }
    }
    if (state == HOT) {
      // PRINT("state is hot... ");
      // PRINTLN(now - stop_time);
      if ((now - stop_time) > COOLDOWN_DURATION) {
        state = OFF;
      }
    }
  }
};

#endif

#ifndef _CATDOOR_ACCEL_H_
#define _CATDOOR_ACCEL_H_

#include "ADA_LIS3DH.h"
#include "utils.h"

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
  volatile bool data_ready;
  volatile bool new_state;
  volatile state_t state;
  uint8_t jamcounter;

  Accel(void)
      : ADA_LIS3DH(),
        data_ready(false),
        new_state(false),
        state(CLOSED),
        jamcounter(0) {}

  void change_state(state_t s) {
    state = s;
    new_state = true;
    jamcounter = 0;
  }

  void process() {
    // read get the lastest data and at the same time clears the interupt signal
    read();
    data_ready = false;
    // PRINT(accel[0]);
    // PRINT("\t");
    // PRINT(accel[2]);
    // PRINTLN();
    // compute new state
    if (accel[0] < V_CLOSED && abs(accel[2]) < H_CLOSED) {
      if (state != CLOSED) {
        change_state(CLOSED);
      }
      return;
    }
    if (accel[0] > V_OPENED) {
      if (accel[2] > H_OPENED) {
        if (state != OPEN_IN) {
          change_state(OPEN_IN);
        }
        return;
      }
      if (accel[2] < -H_OPENED) {
        if (state != OPEN_OUT) {
          change_state(OPEN_OUT);
        }
        return;
      }
    }
    if (accel[0] >= V_CLOSED && accel[2] >= H_CLOSED) {
      if (state != AJAR_IN) {
        change_state(AJAR_IN);
      }
      return;
    }
    if (accel[0] < V_JAMMED && accel[2] < H_CLOSED && accel[2] > -H_JAMMED) {
      // only consider jamed state when door is out
      if (state != JAMMED) {
        jamcounter++;
        // Make sure we are totally jammed, and not trough a transition
        if (jamcounter > JAMCOUNTS) {
          change_state(JAMMED);
        }
      }
      return;
    }
    // PRINTLN("Undefined angle: no state change");
  }
};

#endif

#ifndef _CATDOOR_ACCEL_H_
#define _CATDOOR_ACCEL_H_

#include "ADA_LIS3DH.h"
#include "utils.h"

static const char *ACCEL_STATES_NAMES[] = {"CLOSED", "OPEN_IN", "OPEN_OUT",
                                           "AJAR_OUT", "AJAR_IN"};

class Accel : public ADA_LIS3DH {
 public:
  static const int16_t V_CLOSED = -15900;
  static const int16_t H_CLOSED = 500;
  static const int16_t V_JAMMED = -15500;
  static const int16_t H_JAMMED = 3000;
  static const int16_t V_OPENED = -13000;
  static const int16_t H_OPENED = 10000;
  static const uint8_t JAMCOUNTS = 10;  // 1.2s at 10Hz
  static const uint8_t CLOSED_COUNTS = 4;
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
  uint8_t jamcounter;      // "integrate" time jammed is detected
  uint8_t closed_counter;  // just count number of successive closed
  int16_t calib_h;

  Accel(void)
      : ADA_LIS3DH(),
        data_ready(false),
        new_state(false),
        state(CLOSED),
        jamcounter(0),
        closed_counter(0) {
    calib_h = 0;
  }

  void change_state(state_t s) {
    state = s;
    new_state = true;
    jamcounter = 0;
    closed_counter = 0;
  }

  int16_t calibrate() {
    static const size_t loops = 20;
    int32_t h_values = 0;
    for (size_t i = 0; i < loops; i++) {
      while (!new_state) {
        delay(1);
      }
      read();
      new_state = false;
      h_values += accel[2];
      if (accel[2] > V_CLOSED - 100) {
        PRINT("Vertical acceleration outside margin when closed: ");
        PRINT(accel[2]);
        PRINTLN(" !!!");
      }
    }
    calib_h = h_values / loops;
    PRINT("Calibration for horizontal acceleration = ");
    PRINTLN(calib_h);
    return calib_h;
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
    if (accel[0] < V_CLOSED && abs(accel[2]) < H_CLOSED - calib_h) {
      if (state != CLOSED) {
        closed_counter++;
        if (closed_counter > CLOSED_COUNTS) {
          change_state(CLOSED);
        }
      }
      if (jamcounter > 0) jamcounter--;
      return;
    }
    if (accel[0] > V_OPENED) {
      if (accel[2] > H_OPENED) {
        if (state != OPEN_IN) {
          change_state(OPEN_IN);
        }
        if (jamcounter > 0) jamcounter--;
        return;
      }
      if (accel[2] < -H_OPENED) {
        if (state != OPEN_OUT) {
          change_state(OPEN_OUT);
        }
        if (jamcounter > 0) jamcounter--;
        return;
      }
    }
    if (accel[0] >= V_CLOSED && accel[2] >= H_CLOSED - calib_h) {
      if (state != AJAR_IN) {
        change_state(AJAR_IN);
      }
      if (jamcounter > 0) jamcounter--;
      return;
    }
    if (accel[0] < V_JAMMED && accel[2] < -(H_CLOSED - calib_h) &&
        accel[2] > -H_JAMMED) {
      // only consider jamed state when door is out
      // PRINT("accel[0]=");
      // PRINT(accel[0]);
      // PRINT(" / accel[2]=");
      // PRINTLN(accel[2]);
      if (state != JAMMED) {
        jamcounter++;
        // Make sure we are totally jammed, and not trough a transition
        if (jamcounter > JAMCOUNTS) {
          // PRINTLN("detected JAM");
          change_state(JAMMED);
        }
      }
      return;
    } else {
      if (jamcounter > 0) jamcounter--;
    }
    // PRINTLN("Undefined angle: no state change");
  }
};

#endif

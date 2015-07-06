#ifndef TFSM_H
#define TFSM_H


#define TRACK2_SPEED 5
#define TRACK1_SPEED 6
#define TRACK2_DIRECTION 7
#define TRACK1_DIRECTION 8

// needs to be fairly aggressive or the integral term of the control
// loops will accumulate too much error before the robot moves appreciably
#define MIN_VIABLE_SPEED 100
#define MAX_VIABLE_SPEED 255

#define MS_OVER_DELTASPEED 5    // milliseconds per unit, 255 units is max speed

#include <stdint.h>

class TracksFSM {

  int16_t left_current, right_current;
  int16_t left_target, right_target;
  int16_t left_start, right_start;
  
  unsigned long accelStart;
  
  uint8_t state;
  
  enum State {
    STARTUP,
    ACCELERATING,
    HOLDING
  };
  
  public:
  
  TracksFSM();
  
    
  int16_t tween(int16_t current, int16_t target, int16_t max_increment);
  
  void update();
  
  void target(int16_t right, int16_t left);
  
  void write(int16_t right, int16_t left);
  
  inline const int16_t leftCurrent() const {
    return left_current;
  }
  
  inline const int16_t rightCurrent() const {
    return right_current;
  }
};

#endif


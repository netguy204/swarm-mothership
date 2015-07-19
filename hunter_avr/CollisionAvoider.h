/**
 *
 * CollisionAvoider.h
 *
 * Using FRED's two LV-MaxSonar-EZ sensors, we will try to avoid running into
 * things.  We'll let FRED decide when to check for obstructions (e.g., the
 * collision avoidance system can be disabled when FRED is still and/or is
 * performing a scan).
 *
 */


#ifndef _COLLISION_AVOIDER_H_
#define _COLLISION_AVOIDER_H_

#include "LvMaxSonarSensor.h"

// use flags for obstruction detection
#define OBSTRUCTION_LEFT        0x01
#define OBSTRUCTION_RIGHT       0x02
#define OBSTRUCTION_BOTH        (OBSTRUCTION_LEFT | OBSTRUCTION_RIGHT)

#define NO_OBSTRUCTION          0

#define WARNING_DISTANCE_CM     (MINIMUM_LVMAX_RANGE_CM + 10)   // several cm beyond sensor's minimum range


class CollisionAvoider
{
  private:
    LvMaxSonarSensor leftSensor;
    LvMaxSonarSensor rightSensor;
    
  public:
    enum
    {
      IDLE,
      ACTIVE
    };

    uint8_t state;
    
    uint8_t condition;

    // how far are obstructions, if any?
    long leftCm, rightCm;
    
    // Constructor
    CollisionAvoider();
    
    void setEnabled(boolean enabled);

    void update();
};

#endif  // _COLLISION_AVOIDER_H_


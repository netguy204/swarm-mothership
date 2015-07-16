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


class CollisionAvoider
{
  private:
    LvMaxSonarCollisionAvoidanceSensor leftSensor;
    LvMaxSonarCollisionAvoidanceSensor rightSensor;
      
  public:
    enum
    {
      IDLE,
      ACTIVE
    };
    
    uint8_t state;
    
    enum
    {
      UNKNOWN,
      NO_OBSTRUCTION,
      OBSTRUCTION_LEFT,
      OBSTRUCTION_RIGHT,
      OBSTRUCTION_BOTH
    };
    
    uint8_t condition;

    // how far are obstructions, if any?
    long leftCm, rightCm;
    
    // Constructor
    CollisionAvoider();
    
    void setEnabled(boolean enabled);

    void update();
};

#endif  // _COLLISION_AVOIDER_H_


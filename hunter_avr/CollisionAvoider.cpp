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

#include "CollisionAvoider.h"


// Constructor
CollisionAvoider::CollisionAvoider()
{
  state = IDLE;
  condition = NO_OBSTRUCTION;
}

void CollisionAvoider::setEnabled(boolean enabled)
{
  if(enabled)
  {
    leftSensor.enableReadPin();
    //rightSensor.enableReadPin();  // already tied to left sensor
    state = ACTIVE;
    condition = NO_OBSTRUCTION;  // until we see something...

  }
  else
  {
    leftSensor.disableReadPin();
    //rightSensor.disableReadPin();  // already tied to left sensor
    state = IDLE;
    condition = NO_OBSTRUCTION;  // we're probably not driving right now
  }
}

void CollisionAvoider::update()
{
  // if we're active, then let's see if anything is in our way
  if(state == ACTIVE)
  {
    condition = NO_OBSTRUCTION;  // ever the optimist!
    
    leftCm = leftSensor.getDistanceCm();
    rightCm = rightSensor.getDistanceCm();
    
    // let's see what condition my condition is in...
    if(leftCm <= WARNING_DISTANCE_CM)
    {
      condition |= OBSTRUCTION_LEFT;
    }
    
    if(rightCm <= WARNING_DISTANCE_CM)
    {
      condition |= OBSTRUCTION_RIGHT;
    }
  }
}



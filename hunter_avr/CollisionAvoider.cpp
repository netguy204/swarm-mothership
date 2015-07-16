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
  leftSensor.setMinimumRange(DEFAULT_MINIMUM_RANGE_CM);
  rightSensor.setMinimumRange(DEFAULT_MINIMUM_RANGE_CM);
  state = IDLE;
  condition = UNKNOWN;
}

void CollisionAvoider::setEnabled(boolean enabled)
{
  if(enabled)
  {
    leftSensor.enableReadPin();
    rightSensor.enableReadPin();
    state = ACTIVE;
  }
  else
  {
    leftSensor.disableReadPin();
    rightSensor.disableReadPin();
    state = IDLE;
    condition = UNKNOWN;
  }
}

void CollisionAvoider::update()
{
  // if we're active, then let's see if anything is in our way
  if(state == ACTIVE)
  {
    leftCm = leftSensor.getDistanceCm();
    rightCm = rightSensor.getDistanceCm();
    
    if(leftSensor.isTooClose() && rightSensor.isTooClose())
    {
      condition = OBSTRUCTION_BOTH;
    }
    else if(leftSensor.isTooClose())
    {
      condition = OBSTRUCTION_LEFT;
    }
    else if(rightSensor.isTooClose())
    {
      condition = OBSTRUCTION_RIGHT;
    }
    else  // all clear!
    {
      condition = NO_OBSTRUCTION;
    }
  }
}



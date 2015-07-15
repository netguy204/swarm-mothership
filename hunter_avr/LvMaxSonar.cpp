/**
 *
 * LvMaxSonar.cpp
 *
 * Code for reading the collision-avoidance sonar (LV-MaxSonar-EZ), based on sample
 *.code by Bruce Allen, dated 23/07/09.
 * The speed of sound at sea level is 29.3866996 usec/cm (74.6422169 usec/inch),
 * but the pulses we'll be reading from the LV-MaxSonar-EX are 147 usec/inch,
 * according to the datasheet (XXX - This value should be more like 149.28 usec/inch, right?).
 *
 * Datasheet:  http://maxbotix.com/documents/LV-MaxSonar-EZ_Datasheet.pdf
 *
 */

#include "LvMaxSonar.h"

// LvMaxSonar constructor
LvMaxSonar::LvMaxSonar()
{
  // nothing more to do here, move along...
}

void LvMaxSonar::setPin(int pinIn)
{
  pwPin = pinIn;
}

long LvMaxSonar::getDistanceCm()
{
  long pulseDuration, cm;
  
  pinMode(pwPin, INPUT);

  // Read the pulse from the LV-MaxSonar-EZ.
  pulseDuration = pulseIn(pwPin, HIGH, CM_MAX_RANGE_USEC_TIMEOUT);

  // ~147 usec/inch, times 2.54 to get cm
  //long cm = (pulseDuration/SCALE_USEC_PER_INCH) * CM_PER_INCH;
  cm = (pulseDuration/SCALE_USEC_PER_CM);
  
  lastMeasurementCm = cm;
    
  return cm;
}

long LvMaxSonar::getLastMeasurementCm()
{
  return lastMeasurementCm;
}


// LvMaxSonarCollisionAvoidance constructor
LvMaxSonarCollisionAvoidance::LvMaxSonarCollisionAvoidance(int pinIn,
                                                           int minRangeCm)
  : minRangeCm(minRangeCm)
{
  // nothing more to do here, move along...
  setPin(pinIn);
}

long LvMaxSonarCollisionAvoidance::getDistanceCm()
{
  long cm = LvMaxSonar::getDistanceCm();

  if(cm <= minRangeCm)
    tooClose = true;
  else
    tooClose = false;
    
  return cm;
}

bool LvMaxSonarCollisionAvoidance::isTooClose()
{
  return tooClose;
}



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

// constructor
LvMaxSonar::LvMaxSonar(int pwPin,
                       int minRangeCm)
  : pwPin(pwPin), minRangeCm(minRangeCm)
{
  // nothing more to do here, move along...
}

long LvMaxSonar::getDistanceCm()
{
  long pulseDuration, cm;
  
  pinMode(pwPin, INPUT);

  // Read the pulse from the LV-MaxSonar-EZ.
  pulseDuration = pulseIn(pwPin, HIGH);

  // ~147 usec/inch, times 2.54 to get cm
  //long cm = (pulseDuration/SCALE_USEC_PER_INCH) * CM_PER_INCH;
  cm = (pulseDuration/SCALE_USEC_PER_CM);
  
  lastMeasurementCm = cm;
  if(cm <= minRangeCm)
    tooClose = true;
  else
    tooClose = false;
    
  return cm;
}

long LvMaxSonar::getLastMeasurementCm()
{
  return lastMeasurementCm;
}

bool LvMaxSonar::isTooClose()
{
  return tooClose;
}


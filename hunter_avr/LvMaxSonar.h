/**
 *
 * LvMaxSonar.h
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

#ifndef _LV_MAX_SONAR_H_
#define _LV_MAX_SONAR_H_

#include <Arduino.h>

#define LEFT_COLLISION_AVOID_SENSOR_PIN     22
#define RIGHT_COLLISION_AVOID_SENSOR_PIN    24


#define SCALE_USEC_PER_INCH  149.28      // is 147, according to the datasheet
#define SCALE_USEC_PER_CM     58.77      
#define CM_PER_INCH            2.54

// the maximum range is supposedly 6.45 meters, but we'll say 2...
#define CM_MAX_RANGE_USEC_TIMEOUT (SCALE_USEC_PER_CM * 200)

#define DEFAULT_MINIMUM_RANGE_CM  6    // how close is too close?
#define NO_MINIMUM_RANGE_CM       -1   // no minimum range to worry about


class LvMaxSonar
{
  protected:
    int pwPin;  // pin for reading pulses (pulse of 147usec/inch)
    long lastMeasurementCm = -1;
  
  public:
    LvMaxSonar();
    
    void setPin(int pinNumberIn);

    long getDistanceCm();  // return the distance in cm
    long getLastMeasurementCm();
};


class LvMaxSonarCollisionAvoidance : public LvMaxSonar
{
  private:
    long minRangeCm = DEFAULT_MINIMUM_RANGE_CM;
    bool tooClose = false;

  public:
    LvMaxSonarCollisionAvoidance(int pinNumberIn,
                                 int minRangeCmIn);
    
    long getDistanceCm();  // return the distance in cm, and set the "tooClose" flag as appropriate
                             
    bool isTooClose();  // are we too close to some object?

};

#endif  /* _LV_MAX_SONAR_H_ */


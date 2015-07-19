/**
 *
 * LvMaxSonarSensor.h
 *
 * Code for reading the sonar sensor (LV-MaxSonar-EZ), based on sample
 *.code by Bruce Allen, dated 23/07/09.
 * The speed of sound at sea level is 29.3866996 usec/cm (74.6422169 usec/inch),
 * but the pulses we'll be reading from the LV-MaxSonar-EX are 147 usec/inch,
 * according to the datasheet (XXX - This value should be more like 149.28 usec/inch, right?).
 * 
 * Datasheet:  http://maxbotix.com/documents/LV-MaxSonar-EZ_Datasheet.pdf
 *
 */

#ifndef _LV_MAX_SONAR_SENSOR_H_
#define _LV_MAX_SONAR_SENSOR_H_

#include <Arduino.h>

// pulse width modulation
#define LEFT_COLLISION_AVOID_SENSOR_PW_PIN     22
#define RIGHT_COLLISION_AVOID_SENSOR_PW_PIN    24

// analog (permits cooperation between sensors)
#define LEFT_COLLISION_AVOID_SENSOR_AN_PIN     A14
#define RIGHT_COLLISION_AVOID_SENSOR_AN_PIN    A15
// we can tie all the RX pins on the sensors to this digital pin in order
// to trigger simultaneous readings.  Go high to start getting readings.
#define COLLISION_AVOID_SENSOR_ENABLE_PIN      22

#define SCANNER_SONAR_ENABLE_PIN               24


// pulse width modulation
#define SCALE_USEC_PER_INCH          149.28      // is 147, according to the datasheet
#define SCALE_USEC_PER_CM             58.77      
#define CM_PER_INCH                    2.54

// the maximum range is supposedly 6.45 meters, but we'll say 5...
#define MAXIMUM_LVMAX_RANGE_CM       500
#define CM_MAX_RANGE_USEC_TIMEOUT (SCALE_USEC_PER_CM * MAXIMUM_LVMAX_RANGE_CM)

// for collision avoidance
#define MINIMUM_LVMAX_RANGE_CM        16  // how close is too close for comfort?  Min range is ~15cm...
#define NO_MINIMUM_LVMAX_RANGE_CM     -1  // no minimum range to worry about


class LvMaxSonarSensor
{
  public:
    typedef enum
    {
      PWM_MODE,
      ANALOG_MODE,
      UNDEFINED
    } SensorMode;
    
  private:
    SensorMode sensorMode = UNDEFINED;
  
  protected:
    // pin for reading pulses (pulse of 149.28usec/inch) or for reading voltages (~9.8mV/inch)
    int readPin;  
    long lastMeasurementCm = -1;
  
  public:
    LvMaxSonarSensor();
    
    void setReadPin(int pinNumberIn, SensorMode sensorModeIn);
    
    // TODO - implement these if we go synchronized analog:
    void enableReadPin();
    void disableReadPin();
        
    long getDistanceCm();  // return the distance in cm
    long getLastMeasurementCm();
};


#endif  /* _LV_MAX_SONAR_SENSOR_H_ */


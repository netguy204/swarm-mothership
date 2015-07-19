/**
 *
 * LvMaxSonar.cpp
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

#include "LvMaxSonarSensor.h"

// LvMaxSonar constructor
LvMaxSonarSensor::LvMaxSonarSensor()
{
  // nothing more to do here, move along...
}

// set the pin number & the mode (PWM_MODE or ANALOG_MODE)
void LvMaxSonarSensor::setReadPin(int readPinIn, SensorMode sensorModeIn)
{
  readPin = readPinIn;
  sensorMode = sensorModeIn;
}

void LvMaxSonarSensor::enableReadPin()
{
  digitalWrite(COLLISION_AVOID_SENSOR_ENABLE_PIN, HIGH);
}

void LvMaxSonarSensor::disableReadPin()
{
  digitalWrite(COLLISION_AVOID_SENSOR_ENABLE_PIN, LOW);
}

long LvMaxSonarSensor::getDistanceCm()
{
  long cm;
  
  if(sensorMode == PWM_MODE)
  {
    long pulseDuration;
    pinMode(readPin, INPUT);

    // Read the pulse from the LV-MaxSonar-EZ.
    pulseDuration = pulseIn(readPin, HIGH, CM_MAX_RANGE_USEC_TIMEOUT);
    
    if(pulseDuration == 0)
    {
      cm = MAXIMUM_LVMAX_RANGE_CM;
    }
    else
    {
      // ~147 usec/inch, times 2.54 to get cm
      //long cm = (pulseDuration/SCALE_USEC_PER_INCH) * CM_PER_INCH;
      cm = (pulseDuration/SCALE_USEC_PER_CM);
    }
  }
  else  // ANALOG_MODE
  {
    // (Vcc/512)/inch, or ~9.8mV/in for a 5V supply line
    // multiply inches by 2.54 to get cm
    // 
    // TODO - we can pull the RX pin on multiple sensors to get them
    // to read the range simultaneously.  See the datasheet, p.4:
    // http://maxbotix.com/documents/LV-MaxSonar-EZ_Datasheet.pdf
    long cmReading;

    //anVolt = (analogRead(readPin)/2) * 2.54;
    cmReading = analogRead(readPin) * 1.27;  // cm
    
    // use an alpha to smooth things
    if(lastMeasurementCm == -1)
    {
      lastMeasurementCm = cmReading;
    }
    else
    {
      // 0 < alpha < 1
      float alpha = 0.5;
      cm = (long)((alpha * cmReading) + ((1.0 - alpha) * lastMeasurementCm));
    }
  }
  
  lastMeasurementCm = cm;
    
  return cm;
}

long LvMaxSonarSensor::getLastMeasurementCm()
{
  return lastMeasurementCm;
}


/**
 * 
 * scanfsm.cpp
 *
 * Code for performing scanning sweeps.  During such scans, we
 * take ultrasonic distance measurements and we look for the IR
 * beacon.  The results will go back to the mothership once
 * completed.
 *
 */

#include "scanfsm.h"


ScanFSM::ScanFSM()
{
  
  scannerServo.attach(SCANFSM_SERVO_PIN);  // attaches the servo on pin 9 to the servo object
  
  // enable the IR sensor
  pinMode(SCANFSM_IR_PIN, INPUT);
  
  state = SCAN_IDLE;
  
  lastScanStepTime = millis();
}

void ScanFSM::update()
{
  if(state == SCAN_IDLE)
  {
    return;
  }
  else if(state == START_SCAN)
  {
    // get ready for scan... move servo to SCANFSM_SERVO_ANGLE_MIN
    if(servoAngle > SCANFSM_SERVO_ANGLE_MIN)
    {
      if(millis() - lastScanStepTime >= SCAN_STEP_DURATION_MSEC)
      {
        servoAngle--;
        scannerServo.write(servoAngle);
        lastScanStepTime = millis();
      }
    }
    else
    {
      state = SCANNING;
      lastScanStepTime = millis();
    }
  }
  else if(state == SCANNING)
  {
    if(servoAngle >= SCANFSM_SERVO_ANGLE_MAX)
    {
      state == SCAN_COMPLETE;
      newScanResultsWaiting = true;
      lastScanStepTime = millis();
      // next, park the servo at SCANFSM_SERVO_ANGLE_MID
    }
    else
    {
      if(millis() - lastScanStepTime >= SCAN_STEP_DURATION_MSEC)
      {
        long cm = lvMaxSonar.getDistanceCm();
        sonarScanResults[servoAngle - SCANFSM_SERVO_ANGLE_MIN] = cm;
      
        scannerServo.write(servoAngle);
        lastScanStepTime = millis();
      }
    }
  }
  else if(state == SCAN_COMPLETE)
  {
    if(servoAngle > SCANFSM_SERVO_ANGLE_MID)  // park the servo at the mid point
    {
      if(millis() - lastScanStepTime >= SCAN_STEP_DURATION_MSEC)
      {
        servoAngle--;
        scannerServo.write(servoAngle);
        lastScanStepTime = millis();
      }
    }
    else
    {
      state = SCAN_IDLE;
    }
  }
  else
  {
    // unknown state... should not happen
  }


}
  
boolean ScanFSM::foundIrSignal()
{
  uint16_t highpulse = 0; // temporary storage timing
  boolean ir_found = false;
  
  while(highpulse < MAXPULSE)
  {
    // pin is still HIGH?
    ir_found |= !(IRpin_PIN & (1 << SCANFSM_IR_PIN));
    // count off another few microseconds
    highpulse++;
    delayMicroseconds(RESOLUTION);
  }
  
  // Did the pin ever go low?
  if(ir_found)
  {
    Serial.println("IR detected!");
  }
  
  return ir_found;
}


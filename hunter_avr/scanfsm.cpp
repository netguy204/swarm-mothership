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
  
  myservo.attach(SCANNER_SERVO_PIN);  // attaches the servo on pin 9 to the servo object
  
  // enable the IR sensor
  pinMode(SCANNER_IR_PIN, INPUT);
  
  state = SCAN_IDLE;
}

void ScanFSM::update()
{
  if(state == SCAN_IDLE)
  {
    return;
  }
  else if(state == START_SCAN)
  {
    // get ready for scan
    servoAngle = SCANNER_SERVO_ANGLE_MIN;
    scannerServo.write(servoAngle);
    
    // TODO - need to wait until the servo is in position, if it isn't already there
    
  }
  else if(state == SCANNING)
  {
    if(servoAngle >= SCANNER_SERVO_ANGLE_MAX)
    {
      state == SCAN_COMPLETE;
      // park the servo
      servoAngle = SCANNER_SERVO_ANGLE_MIN;
      scannerServo.write(servoAngle);
    }
    else
    {
      long cm = lvMaxSonar.getDistanceCm();
      sonarScanResults[servoAngle - SCANNER_SERVO_ANGLE_MIN] = cm;
      
      scannerServo.write(servoAngle);
      
    }
  }
  else if(state == SCAN_COMPLETE)
  {
    
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
    ir_found |= !(IRpin_PIN & (1 << PIN_SCANNER_IR));
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


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
  Serial.println("In ScanFSM constructor");
  servoAngle = SCANFSM_SERVO_ANGLE_MID;

  state = SCAN_IDLE;
  newScanResultsWaiting = false;
}

void ScanFSM::begin()
{
  Serial.println("In ScanFSM::begin()");

  // park the servo at the midpoint
  Serial.println("Parking the servo");
  scannerServo.attach(SCANFSM_SERVO_PIN);  // attaches the servo on pin 9 to the servo object
  scannerServo.write(servoAngle);
  
  // set up the LV-MaxSonar-EZ
  lvMaxSonarSensor.setReadPin(SCANFSM_SONAR_PIN, LvMaxSonarSensor::PWM_MODE);
  
  // enable the IR sensor
  Serial.println("Enabling the IR pin");
  pinMode(SCANFSM_IR_PIN, INPUT);
  
  Serial.write("Setting lastScanStepTime: ");
  lastScanStepTime = millis();
  Serial.println(lastScanStepTime);
}

void ScanFSM::startScan()
{
  // this will start the scanning process when we next call update()
  // first, we need to move the scanner from the parked (midpoint) position to the right
  // next, scan from right to left
  // when done scanning, park the servo again at the midpoint  
  state = START_SCAN;
}

void ScanFSM::update()
{
  if(state == SCAN_IDLE)
  {
    return;
  }
  else if(state == START_SCAN)
  {
    Serial.print("state = START_SCAN, servoAngle = ");
    Serial.println(servoAngle);
    // get ready for scan... move servo to SCANFSM_SERVO_ANGLE_MIN
    if(servoAngle > SCANFSM_SERVO_ANGLE_MIN)
    {
      if(millis() - lastScanStepTime >= SCAN_STEP_DURATION_MSEC)
      {
        servoAngle--;
        scannerServo.write(servoAngle);
        lastScanStepTime = millis();
        //Serial.print("millis(): ");
        //Serial.println(lastScanStepTime);
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
      state = SCAN_COMPLETE;
      newScanResultsWaiting = true;
      lastScanStepTime = millis();
      //Serial.print("millis(): ");
      //Serial.println(lastScanStepTime);
      // next, park the servo at SCANFSM_SERVO_ANGLE_MID
    }
    else
    {
      if(millis() - lastScanStepTime >= SCAN_STEP_DURATION_MSEC)
      {
        Serial.print("state = SCANNING, servoAngle = ");
        Serial.print(servoAngle);
        long cm = lvMaxSonarSensor.getDistanceCm();
        Serial.print(":  ");
        Serial.print(cm);
        Serial.print("cm   ");
        sonarScanResults[servoAngle - SCANFSM_SERVO_ANGLE_MIN] = cm;
        boolean irFound = foundIrSignal();
        if(irFound)
          Serial.println("IR");
        else
          Serial.println("--");
        irScanResults[servoAngle - SCANFSM_SERVO_ANGLE_MIN] = irFound;
        servoAngle++;
        scannerServo.write(servoAngle);
        lastScanStepTime = millis();
        //Serial.print("millis(): ");
        //Serial.println(lastScanStepTime);
      }
    }
  }
  else if(state == SCAN_COMPLETE)
  {
    Serial.print("state = SCAN_COMPLETE, servoAngle = ");
    Serial.println(servoAngle);
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
      Serial.println("state = SCAN_IDLE");
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


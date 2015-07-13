/**
 * 
 * scanfsm.h
 *
 * Code for performing scanning sweeps.  During such scans, we
 * take ultrasonic distance measurements and we look for the IR
 * beacon.  The results will go back to the mothership once
 * completed.
 *
 */

#ifndef _SCAN_FSM_H_
#define _SCAN_FSM_H_


#include <Arduino.h>
#include <Servo.h>
#include "LvMaxSonar.h"


// TODO - set the pin numbers
#define SCANFSM_SERVO_PIN     111

#define SCANFSM_SONAR_PIN     (123)


// TODO - set these values appropriately
#define SCANFSM_SERVO_ANGLE_MIN     45
#define SCANFSM_SERVO_ANGLE_MAX     135


#if !defined(__AVR_ATmega2560__)
#  define SCANFSM_IR_PIN       2
#  define IRpin_PIN            PIND
#else
// IR sensor (on Mega)
#  define SCANFSM_IR_PIN       4
#  define IRpin_PIN            PINE
#endif

#define RESOLUTION             20    // usec between samples
#define MAXPULSE               1000  // how long we'll look for IR


class ScanFSM {
  
  private:
    uint8_t state;
    
    LvMaxSonar lvMaxSonar();  //(SCANFSM_SONAR_PIN);
    
    Servo scannerServo;  // create servo object to control a servo 
    int servoAngle = SCANFSM_SERVO_ANGLE_MIN;

    void clearScanResults();

    boolean foundIrSignal();
    
  public:
    enum State {
      SCAN_IDLE,
      START_SCAN,
      SCANNING,
      SCAN_COMPLETE
    };
    
    // constructor
    ScanFSM();
    
    // last scan results:
    // each bin will contain a distance in cm
    int sonarScanResults[SCANFSM_SERVO_ANGLE_MAX - SCANFSM_SERVO_ANGLE_MIN];
    // each bin will contain a bool indicating whether we saw the IR beacon or not
    bool irScanResults[SCANFSM_SERVO_ANGLE_MAX - SCANFSM_SERVO_ANGLE_MIN];
    
    void update();
};


#endif /* _SCAN_FSM_H_ */

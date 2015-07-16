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
#include "LvMaxSonarSensor.h"


// set the pin numbers
#define SCANFSM_SERVO_PIN     A2
#define SCANFSM_SONAR_PIN     26

// we'll have to look for a 38kHz modulated IR signal
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

// set these values so we don't turn too far (90 degrees seems reasonable)
#define SCANFSM_SERVO_ANGLE_MIN     45
#define SCANFSM_SERVO_ANGLE_MAX     135
#define SCANFSM_SERVO_ANGLE_MID (SCANFSM_SERVO_ANGLE_MIN + ((SCANFSM_SERVO_ANGLE_MAX - SCANFSM_SERVO_ANGLE_MIN) / 2))

// how fast should we scan?
#define SCAN_STEP_DURATION_MSEC     5


class ScanFSM {
  
  private:
    uint8_t state;
    
    // when did we do the last scan sample?
    unsigned long lastScanStepTime;
    
    LvMaxSonarSensor lvMaxSonarSensor;  //(SCANFSM_SONAR_PIN);
    
    Servo scannerServo;  // create servo object to control a servo 
    int servoAngle = SCANFSM_SERVO_ANGLE_MIN;

    void clearScanResults();

    boolean foundIrSignal();
    
  public:
    enum State {
      SCAN_IDLE,
      START_SCAN,    // move servo from the midpoint to the right
      SCANNING,      // scan from right to left
      SCAN_COMPLETE  // park the servo at the midpoint
    };
    
    // constructor
    ScanFSM();
    
    void begin();
    void startScan();
    
    // last scan results:
    bool newScanResultsWaiting = false;
    
    // each bin will contain a distance in cm
    int sonarScanResults[SCANFSM_SERVO_ANGLE_MAX - SCANFSM_SERVO_ANGLE_MIN];
    // each bin will contain a bool indicating whether we saw the IR beacon or not
    bool irScanResults[SCANFSM_SERVO_ANGLE_MAX - SCANFSM_SERVO_ANGLE_MIN];
    
    void update();
};


#endif /* _SCAN_FSM_H_ */

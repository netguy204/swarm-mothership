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


#define SCANNER_SERVO_PIN     333

#define SCANNER_SONAR_PIN     111
#define SCANNER_IR_PIN        222   

#define SCANNER_SERVO_ANGLE_MIN     45
#define SCANNER_SERVO_ANGLE_MAX     135


class ScanFSM {
  
  private:
    uint8_t state;
    
    LvMaxSonar lvMaxSonar(SCANNER_SONAR_PIN);
    
    Servo scannerServo;  // create servo object to control a servo 
    int servoAngle = SCANNER_SERVO_ANGLE_MIN;

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
    int sonarScanResults[SCANNER_SERVO_ANGLE_MAX - SCANNER_SERVO_ANGLE_MIN];
    // each bin will contain a bool indicating whether we saw the IR beacon or not
    bool irScanResults[SCANNER_SERVO_ANGLE_MAX - SCANNER_SERVO_ANGLE_MIN];
    
    void update();
};


#endif /* _SCAN_FSM_H_ */

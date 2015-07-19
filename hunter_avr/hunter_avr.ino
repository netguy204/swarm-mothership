/**
 */

#include <Servo.h>
#include <limits.h>
#include <espduino.h>
#include "custom_rest.h"
#include <ArduinoJson.h>
#include <Wire.h>

#include "messages.h"
#include "pfsm.h"
#include "gpsfsm.h"
#include "mfsm.h"
#include "CollisionAvoider.h"
#include "scanfsm.h"
#include "tfsm.h"
#include "pid.h"

#include "common.h"
#include "swarm_config.h"
#include "ApplicationMonitor.h"

#define VBATTERY A0

long readVcc() {
  // Read 1.1V reference against AVcc
  // set the reference to Vcc and the measurement to the internal 1.1V reference
  #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
    ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
    ADMUX = _BV(MUX5) | _BV(MUX0);
  #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
    ADMUX = _BV(MUX3) | _BV(MUX2);
  #else
    ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
  #endif

  delay(1); // Wait for Vref to settle
  ADCSRA |= _BV(ADSC); // Start conversion
  while (bit_is_set(ADCSRA,ADSC)); // measuring

  uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
  uint8_t high = ADCH; // unlocks both

  long result = (high<<8) | low;

  result = 1125300L / result; // Calculate Vcc (in mV); 1125300 = 1.1*1023*1000
  return result; // Vcc in millivolts
}

Watchdog::CApplicationMonitor ApplicationMonitor;

ProtocolFSM pfsm(Serial1, "swarmiest", "swarmiest", "192.168.168.100", 8080);
//ProtocolFSM pfsm(Serial1, "swarmiest", "swarmiest", "192.168.168.3", 8080);
GPSFSM gpsfsm(Serial3, &Serial);

TracksFSM tfsm;
MagFSM mfsm(0x1E); // I2C 7bit address of HMC5883

CollisionAvoider collisionAvoider;
ScanFSM scanfsm;

Profiler<8> pf;

void setup() {
  Serial.begin(115200);

  // dump any crash data we're holding onto
  Serial.println(F("READY"));

  ApplicationMonitor.Dump(Serial);
  ApplicationMonitor.EnableWatchdog(Watchdog::CApplicationMonitor::Timeout_4s);

  Serial1.begin(19200);
  Serial3.begin(9600);
  Wire.begin();

  gpsfsm.begin();
  scanfsm.begin();

  pinMode(VBATTERY, INPUT);
}

// FRED --> Free-Roving Exploration Device ;-)
class FRED {
  PID hdgPid;

  public:

  enum {
    STARTUP,
    IDLE,
    COMMAND_COMPLETE,
    COMMAND_FAILED,
    START_SET_HEADING,
    RUNNING_SET_HEADING,
    START_DRIVE,
    RUNNING_DRIVE,
    START_SCANNING,
    SCANNING,
    MAX
  };

  uint8_t state;
  double hdgSetPoint, hdgInput, hdgOutput;
  double hdgLongTerm;  // we have a desired heading, but we might have to deviate temporarily
  unsigned long delay_end;
  unsigned long command_timeout;
  uint8_t drive_speed;

  void setDelay(long ms) {
    delay_end = millis() + ms;
  }

  bool delayExpired() {
    return millis() >= delay_end;
  }

  // PID values (2, 5, 1) are probably good... may need to investigate I (integral) term
  FRED()
  : hdgPid(&hdgInput, &hdgOutput, &hdgSetPoint, 2, 5, 1, PID::DIRECT), delay_end(0), command_timeout(5000) {

    hdgInput = 0;
    hdgSetPoint = 0;
    hdgLongTerm = 0;
    
  }

  void update(MagFSM& mfsm, TracksFSM& tfsm) {
    if(state == STARTUP) {
      collisionAvoider.setEnabled(false);
      state = IDLE;
    }

    if(state == START_SET_HEADING || state == START_DRIVE) {
      collisionAvoider.setEnabled(true);
      collisionAvoider.update(); // kick through 1 update to ensure a valid condition
      
      // configure the controller to maintain heading
      hdgPid.SetOutputLimits(-255, 255);
      hdgPid.SetMode(PID::AUTOMATIC);
      hdgPid.SetSampleTime(200);
    }

    if(state == RUNNING_SET_HEADING || state == RUNNING_DRIVE) {
      // update the controller to create a heading correction signal
      if(mfsm.updated_data) {
        hdgInput = mfsm.heading();
        // handle angle wraparound
        double error = hdgInput - hdgSetPoint;
        if(error > 180) {
          hdgInput -= 360;
        } else if(error < -180) {
          hdgInput += 360;
        }

        switch(collisionAvoider.condition)
        {
          case NO_OBSTRUCTION:
            // if FRED is not on the long term heading, try to get back to
            // that (more slowly than the initial turns to avoid collisions)
            if(hdgSetPoint != hdgLongTerm)
            {
              if(hdgSetPoint > hdgLongTerm)
              {
                hdgSetPoint -= 15;
              }
              else
              {
                hdgSetPoint += 15;
              }
            }
            break;
          case OBSTRUCTION_BOTH:
            // We are blocked up ahead.  Since we can't see backwards,
            // we just stop the FRED and change the state to COMMAND_COMPLETE
            // we may need the mothership's help
            tfsm.write(0, 0);
            collisionAvoider.setEnabled(false);
            state = COMMAND_COMPLETE;
            break;
          case OBSTRUCTION_LEFT:
            // veer right
            hdgSetPoint += 30;
            break;
          case OBSTRUCTION_RIGHT:
            // veer left
            hdgSetPoint -= 30;
            break;
          default: // should not get here
            break;
        }  // JMA

        hdgPid.Compute();
      }
    }

    if(state == START_SET_HEADING) {
      Serial.println(F("START_SET_HEADING"));
      setDelay(command_timeout);
      state = RUNNING_SET_HEADING;
    }

    if(state == RUNNING_SET_HEADING) {
      if(abs(hdgInput - hdgSetPoint) < HEADING_PRECISION) {
        Serial.println(F("finished SET_HEADING"));
        hdgPid.SetMode(PID::MANUAL);
        tfsm.write(0, 0);
        collisionAvoider.setEnabled(false);
        state = COMMAND_COMPLETE;
      } else {
        int16_t output = hdgOutput;
        tfsm.write(output, -output);
      }

      if(delayExpired()) {
        Serial.println(F("failed SET_HEADING"));
        tfsm.write(0, 0);
        collisionAvoider.setEnabled(false);
        state = COMMAND_FAILED;
      }
    }

    if(state == START_DRIVE) {
      Serial.println(F("START_DRIVE"));
      setDelay(command_timeout);
      state = RUNNING_DRIVE;
    }

    if(state == RUNNING_DRIVE) {
      if(delayExpired()) {
        tfsm.write(0, 0);
        collisionAvoider.setEnabled(false);
        state = COMMAND_COMPLETE;
      } else {
        double t1, t2;
        // drive speed = (t1 + t2) / 2
        // turn rate = (t1 - t2)
        // t1 = 1/2 * (2*speed + turn)
        // t2 = 1/2 * (2*speed - turn)
        t1 = 0.5 * (2.0 * drive_speed + hdgOutput);
        t2 = 0.5 * (2.0 * drive_speed - hdgOutput);

        // clamp
        if(abs(t1) > 255 || abs(t2) > 255) {
          double mx = max(abs(t1), abs(t2));
          t1 = (t1 / mx) * 255;
          t2 = (t2 / mx) * 255;
        }
        tfsm.write((int16_t)t1, (int16_t)t2);
      }
    }
    
    if(state == START_SCANNING) {
      collisionAvoider.setEnabled(false);
      scanfsm.startScan();
      state = SCANNING;
    }
    
    if(state == SCANNING) {
      if(scanfsm.newScanResultsWaiting == true)
      {
        // XXX - send the data arrays to the mothership & exit SCANNING state
        
        state = IDLE;
      }
    }
  }
};

FRED fred;
SensorStatus ss;

ScanResults scanResults;


void loop() {
  ApplicationMonitor.IAmAlive();
  // remember most of our protocol state if we crash
  ApplicationMonitor.SetData(pfsm.state | (pfsm.wifi_connected << 8) | (pfsm.status_pending << 9) | (pfsm.command_valid << 10) | (pfsm.command_complete << 11));

  pf.start();
  ProtocolFSM::ProtocolState old_state = pfsm.state;
  uint8_t cstate = collisionAvoider.state;
  uint8_t ccond = collisionAvoider.condition;
  
  pfsm.update(); pf.mark(1);
  gpsfsm.update(); pf.mark(2);
  mfsm.update(); pf.mark(3);
  tfsm.update(); pf.mark(4);
  collisionAvoider.update(); pf.mark(5);
  scanfsm.update(); pf.mark(6);

  fred.update(mfsm, tfsm);

  // if the executor has completed the command, ack
  if(fred.state == FRED::COMMAND_COMPLETE || fred.state == FRED::COMMAND_FAILED) {
    fred.state = FRED::IDLE;
  }

  // if we have a new command available and we're idle, push it to the executor
  if(fred.state == FRED::IDLE && pfsm.command_valid && !pfsm.command_complete) {
    // start setting heading
    if(pfsm.command.command == DriveCommand::SET_HEADING) {
      fred.hdgSetPoint = pfsm.command.payload.heading.heading;
      fred.command_timeout = pfsm.command.payload.heading.duration; // implicit 5 second timeout on heading changes
      fred.state = FRED::START_SET_HEADING;
    } else if(pfsm.command.command == DriveCommand::DRIVE) {
      fred.hdgSetPoint = pfsm.command.payload.drive.heading;
      fred.hdgLongTerm = fred.hdgSetPoint;  // we should always return to the long-term heading
      fred.drive_speed = pfsm.command.payload.drive.speed;
      fred.command_timeout = pfsm.command.payload.drive.duration;
      fred.state = FRED::START_DRIVE;
    } else if(pfsm.command.command == DriveCommand::SCAN) {
      // FRED will start scanning (sonar ranging & looking for IR)
      fred.state = FRED::START_SCANNING;
    }
    // ack optimistically so protocol can fetch the next command
    pfsm.command_complete = true;
  }

  // scan results go out at a higher priority than general status results
  // since they are explicitly user requested
  if(pfsm.state == ProtocolFSM::IDLE && scanfsm.newScanResultsWaiting == true) {
    Serial.println("Uploading scan results to the server"); 
    scanResults.heading = mfsm.heading();
    scanResults.gps_time_ms = gpsfsm.lastTime;

    scanResults.message_time_ms = gpsfsm.gmillis();
    scanResults.lat = gpsfsm.lat;
    scanResults.lon = gpsfsm.lon;
    scanResults.gps_fix_state = gpsfsm.status;
    for(int i = 0; i < (SCANFSM_SERVO_ANGLE_MAX - SCANFSM_SERVO_ANGLE_MIN); i++)
    {
      scanResults.sonarScanResults[i] = scanfsm.sonarScanResults[i];
      scanResults.irScanResults[i] = scanfsm.irScanResults[i];
    }
    
    pfsm.sendScanResults(scanResults);
    scanfsm.newScanResultsWaiting = false;
  }
  
  if(pfsm.state == ProtocolFSM::IDLE && (gpsfsm.updated_ll || mfsm.updated_data)) {
    gpsfsm.toENU(ss.enu_cm, gpsfsm.ecef_pos);
    ss.ecef_pos_cm = gpsfsm.ecef_pos;
    ss.ecef_vel_cmps = gpsfsm.ecef_vel;
    ss.magnetometer = Vector<int16_t>(mfsm.x, mfsm.y, mfsm.z);
    ss.heading = mfsm.heading();
    ss.gps_time_ms = gpsfsm.lastTime;

    ss.message_time_ms = gpsfsm.gmillis();
    ss.lat = gpsfsm.lat;
    ss.lon = gpsfsm.lon;
    ss.gps_fix_state = gpsfsm.status;
    ss.vbattery = (5.0 * analogRead(VBATTERY)) / 1023.0;
    ss.vin = readVcc() / 1000.0;

    pfsm.sendStatus(ss);
  }

  gpsfsm.ackLatLon();
  mfsm.ackData();
  gpsfsm.ackECEF();

  if(old_state != pfsm.state) {
#if(1 || VERBOSE_DBG)
    Serial.print(ProtocolFSM::StateStr[old_state]);
    Serial.print(F(" => "));
    Serial.println(ProtocolFSM::StateStr[pfsm.state]);
#endif
  }

  pf.mark(7);

  if(pf.nstarts == 10000) {
    pf.report();
    pf.reset();
  }
  
  if(cstate != collisionAvoider.state || ccond != collisionAvoider.condition) {
    Serial.print(F("collision state = "));
    Serial.print(collisionAvoider.state);
    Serial.print(F(", condition = "));
    Serial.println(collisionAvoider.condition);
  }
  

  // if the fsm wants to sleep, do it now since we have nothing
  // left to do with our time
  //if(!pfsm.delayComplete()) {
  //  delay(pfsm.pendingDelay());
  //}
}

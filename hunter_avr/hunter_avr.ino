/**
 */

#include <limits.h>
#include <espduino.h>
#include "custom_rest.h"
#include <ArduinoJson.h>
#include <Wire.h>

#include "messages.h"
#include "pfsm.h"
#include "gpsfsm.h"
#include "mfsm.h"
#include "tfsm.h"
#include "pid.h"

#include "common.h"
#include "swarm_config.h"



ProtocolFSM pfsm(Serial1, "NETGEAR05", "rusticboat531", "192.168.168.2", 8080);
GPSFSM gpsfsm(Serial3, &Serial);

TracksFSM tfsm;
// ComplexCompensation mc(MAG_FORWARD, MAG_REVERSE, MAG_RIGHT, MAG_LEFT, tfsm);
SimpleCompensation mc(MAG_CENTER);
MagFSM mfsm(0x1E, mc); // I2C 7bit address of HMC5883

Profiler<5> pf;

void setup() {
  Serial.begin(115200);
  Serial1.begin(19200);
  Serial3.begin(9600);
  Wire.begin();
  
  gpsfsm.begin();
}


// FRED --> Free-Roving Exploration Device ;-)
class NorthFindingFred {
  enum {
    STARTUP,
    SEARCHING,
    HOLDING
  };
  
  PID pid;
  uint8_t state;
  
  public:
  double p_setPoint, p_input, p_output;
  // PID values (2, 5, 1) are probably good... may need to investigate I (integral) term
  NorthFindingFred()
  : state(STARTUP), pid(&p_input, &p_output, &p_setPoint, 2, 5, 1, PID::DIRECT) {
  }
  
  void update(MagFSM& mfsm, TracksFSM& tfsm) {
    if(state == STARTUP) {
      p_input = 0;
      p_setPoint = 0;
      pid.SetOutputLimits(-255, 255);
      pid.SetMode(PID::AUTOMATIC);
      pid.SetSampleTime(200);      
      state = SEARCHING;
    }
    
    if(state == SEARCHING || state == HOLDING) {
      if(mfsm.updated_data) {
        p_input = mfsm.heading();
        double error = p_input - p_setPoint;
        if(error > 180) {
          p_input -= 360;
        } else if(error < -180) {
          p_input += 360;
        }
        
        if(abs(p_input - p_setPoint) < HEADING_PRECISION) {
          pid.SetMode(PID::MANUAL);
          tfsm.write(0, 0);
          state = HOLDING;
        } else {
          //Serial.println(p_output);
          
          // proportional integral derivative
          pid.SetMode(PID::AUTOMATIC);
          pid.Compute();
          int16_t output = p_output;
          //Serial.print("output:  ");
          //Serial.println(output);
          // tfsm.write(-output, output); <-- is this correct?
          tfsm.write(output, -output);
          state = SEARCHING;
        }
      }        
    }
  }
};  // NorthFindingFred class


class MagnetometerCalibration {
  enum {
    STARTUP,
    STOPPED,
    FORWARD,
    REVERSE,
    TURN_RIGHT,
    TURN_LEFT,
    MAX,
    HOLD
  };
  
  uint8_t state;
  uint8_t nmeasurements;
  
  public:
  MagnetometerCalibration()
  : state(STARTUP) {
  }
  
  void update(MagFSM& mfsm, TracksFSM& tfsm) {
    if(state == STARTUP) {
      nmeasurements = 0;
      Serial.println("N,Test,x,y,z");
      state = STOPPED;
    }
    
    // initialization
    if(nmeasurements == 0) {
      if(state == STOPPED) {
        tfsm.write(0, 0);
      } else if(state == FORWARD) {
        tfsm.write(255, 255);
      } else if(state == REVERSE) {
        tfsm.write(-255, -255);
      } else if(state == TURN_RIGHT) {
        tfsm.write(-255, 255);
      } else if(state == TURN_LEFT) {
        tfsm.write(255, -255);
      }
    }
    
    // get measurements
    if(state >= STOPPED && state <= TURN_LEFT) {
      if(mfsm.updated_data) {
#if(VERBOSE_DBG)
        Serial.print(nmeasurements);
        Serial.print(",");
        Serial.print(state);
        Serial.print(",");
        Serial.print(mfsm.x);
        Serial.print(",");
        Serial.print(mfsm.y);
        Serial.print(",");
        Serial.println(mfsm.z);
#endif
        mfsm.ackData();
        nmeasurements++;
        
        // cycle to next state
        if(nmeasurements == 255) {
          state = state + 1;
          if(state == MAX) state = STOPPED;
          nmeasurements = 0;
        }
      }
    }
  }
};

NorthFindingFred nff;
MagnetometerCalibration mcal;
SensorStatus ss;

void loop() {
  pf.start();
  ProtocolFSM::ProtocolState old_state = pfsm.state; 
  pfsm.update(); pf.mark(1);
  gpsfsm.update(); pf.mark(2);
  mfsm.update(); pf.mark(3);
  tfsm.update(); pf.mark(4);


  nff.update(mfsm, tfsm);
  //mcal.update(mfsm, tfsm);
  
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
    pfsm.sendStatus(ss);
  }

  
  gpsfsm.ackLatLon();
  mfsm.ackData();
  gpsfsm.ackECEF();
 
  if(old_state != pfsm.state) {
#if(1 || VERBOSE_DBG)
    Serial.print(ProtocolFSM::StateStr[old_state]);
    Serial.print(" => ");
    Serial.println(ProtocolFSM::StateStr[pfsm.state]);
#endif
  }
  
  // if we have a command and that command is SET_HEADING then
  // adjust the setpoint for north finding fred
  if(pfsm.command_valid && !pfsm.command_complete) {
    if(pfsm.command.command == DriveCommand::SET_HEADING) {
      nff.p_setPoint = pfsm.command.payload.heading.heading;
    }
    pfsm.command_complete = true;
  }
  
  pf.mark(5);
  
  if(pf.nstarts == 10000) {
    pf.report();
    pf.reset();
  }
  
  // if the fsm wants to sleep, do it now since we have nothing
  // left to do with our time
  //if(!pfsm.delayComplete()) {
  //  delay(pfsm.pendingDelay());
  //}
}

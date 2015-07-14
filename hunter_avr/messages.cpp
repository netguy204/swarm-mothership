#include "messages.h"

#include "swarm_config.h"

SensorStatus::SensorStatus()
  : lat(0), lon(0), heading(0) {
}

void SensorStatus::toJson(JsonObject& report) {
  report["gtime"] = gps_time_ms;
  report["mtime"] = message_time_ms;
  
  report["lat"] = lat;
  report["long"] = lon;
  report["hdg"] = heading;
  report["pid"] = swarmID();
  
  report["x"] = ecef_pos_cm.x;
  report["y"] = ecef_pos_cm.y;
  report["z"] = ecef_pos_cm.z;
  
  report["vx"] = ecef_vel_cmps.x;
  report["vy"] = ecef_vel_cmps.y;
  report["vz"] = ecef_vel_cmps.z;
  
  report["mx"] = magnetometer.x;
  report["my"] = magnetometer.y;
  report["mz"] = magnetometer.z;
  
  report["gstate"] = gps_fix_state;
  
  report["e"] = enu_cm.x;
  report["n"] = enu_cm.y;
  report["u"] = enu_cm.z;
  
  report["vbattery"] = vbattery;
  report["vin"] = vin;
}

bool DriveCommand::fromJson(JsonObject& cmd) {
  // get the command type 
  if(!cmd.containsKey("type")) return false;
  
  const char* type = cmd["type"];
  if(strcmp(type, "SET_HEADING") == 0) {
    command = SET_HEADING;
  } else if(strcmp(type, "DRIVE") == 0) {
    command = DRIVE;
  } else if(strcmp(type, "SCAN") == 0) {
    command = SCAN;
  } else { 
    Serial.print("unrecognized type ");
    Serial.println(type);
    return false;
  }
  
  if(!cmd.containsKey("cid") || !cmd.containsKey("pid") || !cmd.containsKey("duration")) {
    return false;
  }
  
  cid = cmd["cid"];
  pid = cmd["pid"];
  duration = cmd["duration"];
  
  // get the command parameters
  if(command == DRIVE) {
    if(!cmd.containsKey("speed") || !cmd.containsKey("heading")) return false;
    
    payload.drive.speed = cmd["speed"];
    payload.drive.heading = cmd["heading"];
    return true;
  } else if(command == SET_HEADING) {
    if(!cmd.containsKey("heading")) return false;
    
    payload.heading.heading = cmd["heading"];
    return true;
  } else if(command == SCAN) {
    // no parameters for scanning...?
    return true;
  } else {
    return false;
  }
 
}

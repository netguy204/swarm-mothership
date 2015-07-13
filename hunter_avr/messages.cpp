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
  report["pid"] = SWARM_ID;
  
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
}

bool DriveCommand::fromJson(JsonObject& cmd) {
  // get the command type 
  if(!cmd.containsKey("type")) return false;
  if(strcmp(cmd["type"], "DRIVE") == 0) {
    command = DRIVE;
  } else if(strcmp(cmd["type"], "SPIRAL_OUT") == 0) {
    command = SPIRAL_OUT;
  } else if(strcmp(cmd["type"], "SET_HEADING") == 0) {
    command = SET_HEADING;
  } else if(strcmp(cmd["type"], "SCAN") == 0) {
    command = SCAN;
  } else {
    const char* type = cmd["type"];
    Serial.print("unrecognized type ");
    Serial.println(type);
    return false;
  }
  
  cid = cmd["cid"];
  pid = cmd["pid"];
  
  // get the command parameters
  if(command == DRIVE) {
    // get speeds
    if(!cmd.containsKey("sl") || !cmd.containsKey("sr")) return false;
    
    payload.drive.speed_left = cmd["sl"];
    payload.drive.speed_right = cmd["sr"];
    return true;
  } else if(command == SPIRAL_OUT) {
    if(!cmd.containsKey("erpl")) return false;
    
    payload.spiral_out.extra_radius_per_loop = cmd["erpl"];
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

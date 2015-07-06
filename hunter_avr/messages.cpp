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
  if(!cmd.containsKey("cmd")) return false;
  command = cmd["cmd"];
  
  if(command == DRIVE) {
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
  } else {
    return false;
  }
}

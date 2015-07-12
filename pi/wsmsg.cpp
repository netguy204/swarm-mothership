#include <asm/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "wsmsg.h"


bool WebServiceMsg::fromJson(JsonObject& obj) {

  if (!obj.containsKey("pid") || !obj.containsKey("cid") || !obj.containsKey("speed") || !obj.containsKey("angle")) {
    return false;
  }
  pid = obj["pid"];
  cid = obj["cid"];
  speed = obj["speed"];
  angle = obj["angle"];
  if (obj.containsKey("completed")) {
    completed = obj["completed"];
  }
  else {
    completed = false;
  }
  return true;
}
void WebServiceMsg::toJson(JsonObject& obj) {

  obj["pid"] = pid;
  obj["cid"] = cid;
  obj["speed"] = speed;
  obj["angle"] = angle;
  obj["completed"] = completed;

}

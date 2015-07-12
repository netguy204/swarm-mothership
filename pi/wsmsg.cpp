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

void WebServiceMsg::putCmdStatus(long cid, bool status) {
  
  StaticJsonBuffer<200> jsonBuffer;
  JsonObject& root = jsonBuffer.createObject();
  root["cid"] = 0; // mothership
    
  if (status) {
    root["complete"] = true; 
  } else {
    root["complete"] = false; 
  }
  

}

/* 
cmd json syntax:
if type=DRIVE speed, heading
*/
Message WebServiceFSM::pullQueuedCmd() {

  StaticJsonBuffer<200> jsonBuffer;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"pid\":0}");
  CURLcode res = curl_easy_perform(curl);
 
  if(res != CURLE_OK) {
    fprintf(stderr, "WebQ: Failed to access %s: %s\n", "localhost",
        curl_easy_strerror(res));
    //return 1;
  }
  fprintf(stderr, "WebQ: Connection successful\n");
  fprintf(stderr, "%s\n", cmdFromQueue);
  JsonObject& root = jsonBuffer.parseObject(cmdFromQueue);

  if (!root.success()) {
    printf("json parsing failed.\n");
    // should return?
  }

  long pid = root["pid"];
  uint16_t cid = root["cid"];

  
  //  long pid = root["properties"]["id"];
  //  long cmdId = root["cid"]; // UNCOMMENT AFTER ADDED TO WEBSERVICE
  fprintf(stderr,"pid =  %ld\n",pid);
  Message msg;
  // TEMP
  int8_t speed_ival = 1;
  int8_t angle_ival = 1;
  int id = 0;
  // end TEMP
  messageSignedInit(&msg, COMMAND_SET_MOTION, speed_ival, angle_ival, id++);
  return msg;
}

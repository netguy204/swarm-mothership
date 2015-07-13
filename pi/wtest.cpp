
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <curl/curl.h>
#include <ArduinoJson.h>

#include "wsfsm.h"


int main(int argc, char** argv) {
  StaticJsonBuffer<WebServiceFSM::MAX_MSG_SIZE> jsonBuffer;
  auto& root = jsonBuffer.createObject();
  root["pid"] = 15;
  root["cid"] = 12;

  WebServiceFSM wfsm{"http://localhost:8080/commands"};
  wfsm.postJson(root);
  root["cid"] = 11;
  wfsm.putJson(root);
  root["cid"] = 10;
  wfsm.postJson(root);

  return 0;

}

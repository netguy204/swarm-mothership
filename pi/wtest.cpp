
#include <stdio.h>
#include <stdlib.h>
#include "wsfsm.h"
#include <curl/curl.h>

int main(int argc, char** argv) {

  //  long cmd = 1;
  //bool status = true;

  WebServiceFSM wfsm{};
  wfsm.init("http://localhost:8080/update");
  //  wfsm.putCmdStatus(cmd,status);
  wfsm.pullQueuedCmd();
  wfsm.update();

  return 0;
}

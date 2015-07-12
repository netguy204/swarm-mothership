
#include <stdio.h>
#include <stdlib.h>

#include "wsfsm.h"

int main(int argc, char** argv) {

  long cmd = 1;
  bool status = true;

  WebServiceFSM wfsm{"http://localhost:8080/commands"};
  wfsm.putCmdStatus(cmd,status);
  wfsm.pullQueuedCmd();
  wfsm.update();

  return 0;
}

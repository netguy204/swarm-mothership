
#include <stdio.h>
#include <stdlib.h>

#include "wsfsm.h"

int main(int argc, char** argv) {

  WebServiceFSM wfsm{};

  wfsm.init("http://localhost:8080");

  wfsm.update();

  return 0;
}

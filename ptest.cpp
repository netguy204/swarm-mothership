#include <stdio.h>
#include <stdlib.h>
#include "protocol.h"

int main(int argc, char** argv) {
  if(argc != 2) {
    fprintf(stderr, "usage: %s number\n", argv[0]);
    return 1;
  }

  int number = atoi(argv[1]);
  Message msg;
  messageSignedInit(&msg, COMMAND_NOOP, number, 0);
  printf("became = %d\n", messageSignedPayload(&msg));
  return 0;
}

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
  for(int i = -number; i <= number; ++i) {
    messageSignedInit(&msg, COMMAND_NOOP, i, 0);
    printf("became = %d\n", messageSignedPayload(&msg));

    messageSignedInit(&msg, COMMAND_NOOP, i, i, 0);
    printf("became = %d, %d\n", messageSignedPayloadLow(&msg), messageSignedPayloadHigh(&msg));
  }
  return 0;
}

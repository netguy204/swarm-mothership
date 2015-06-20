#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <asm/types.h>
#include <poll.h>

#include "protocol.h"


struct js_event {
  __u32 time;     /* event timestamp in milliseconds */
  __s16 value;    /* value */
  __u8 type;      /* event type */
  __u8 number;    /* axis/button number */
};

#define JS_EVENT_BUTTON         0x01    /* button pressed/released */
#define JS_EVENT_AXIS           0x02    /* joystick moved */
#define JS_EVENT_INIT           0x80    /* initial state of device */

int jsfd;

void joystickInit() {
  jsfd = open ("/dev/input/js0", O_RDONLY);
  if(jsfd < 0);
}

void joystickState(js_event *js) {
  if(jsfd < 0) return;
  static js_event state;

  pollfd pfd;
  pfd.fd = jsfd;
  pfd.events = POLLIN;
  if(poll(&pfd, 1, 0) == 1) {
    read(jsfd, &state, sizeof(struct js_event));
    state.type &= ~JS_EVENT_INIT;
  }

  *js = state;
}

// The PiWeather board i2c address
//#define ADDRESS 0x04


/*
 * Arduino is a pure I2C slave. This client writes commands to it and
 * reads the response after enough time has passed.
 */
// The I2C bus: This is for V2 pi's. For V1 Model B you need i2c-0
//static const char *devName = "/dev/i2c-0";

int main(int argc, char** argv) {

  if (argc != 3) {
    fprintf(stderr, "usage: %s i2c-dev i2c-slave-number\n", argv[0]);
    exit(1);
  }

  const char* devName = argv[1];
  int ADDRESS = atoi(argv[2]);

  printf("I2C: Connecting\n");
  int file;

  if ((file = open(devName, O_RDWR)) < 0) {
    fprintf(stderr, "I2C: Failed to access %d\n", devName);
    exit(1);
  }

  printf("I2C: acquiring buss to 0x%x\n", ADDRESS);

  if (ioctl(file, I2C_SLAVE, ADDRESS) < 0) {
    fprintf(stderr, "I2C: Failed to acquire bus access/talk to slave 0x%x\n", ADDRESS);
    exit(1);
  }

  joystickInit();
  uint8_t id = 0;

  while(true) {
    js_event ev;
    joystickState(&ev);

    double value = 0;
    if(ev.type == JS_EVENT_AXIS && ev.number == 1) {
      printf("ev.value = %d\n", ev.value);
      value = ((double)ev.value) * (1000.0 / 32767.0);
    }

    int16_t ival = (int16_t)value;

    Message _msg;
    uint8_t* msg = (uint8_t*)&_msg;
    messageSignedInit(&_msg, COMMAND_SET_SPEED, ival, id++);

    int nwrote = 0;
    while(nwrote != sizeof(Message)) {
      int wrote = write(file, &msg[nwrote], sizeof(Message) - nwrote);
      printf("wrote = %d\n", wrote);
      if(wrote > 0) {
        nwrote += wrote;
      }
      usleep(10000);
    }

   // usleep(10000);
    Message _reply;
    uint8_t* reply = (uint8_t*)&_reply;

    for(uint ii = 0; ii < 8; ++ii) {
      int nread = 0;
      while(nread != sizeof(Message)) {
        int justRead = read(file, &reply[nread], sizeof(Message) - nread);
        printf("read = %d\n", justRead);
        if(justRead > 0) {
          nread += justRead;
        }
        usleep(10000);
      }
      printf("read %d bytes, type is %d, payload is %d id: %d vs %d\n", nread, _reply.type, messagePayload(&_reply), _reply.id, _msg.id);
      if(_reply.type == COMMAND_SET_SPEED && _reply.id == _msg.id) break;
    }
    //usleep(10000);
  }

  /*
  int arg;

  for (arg = 1; arg < argc; arg++) {
    int val;
    unsigned char cmd[16];

    if (0 == sscanf(argv[arg], "%d", &val)) {
      fprintf(stderr, "Invalid parameter %d \"%s\"\n", arg, argv[arg]);
      exit(1);
    }

    printf("Sending %d\n", val);

    cmd[0] = val;
    if (write(file, cmd, 1) == 1) {

      // As we are not talking to direct hardware but a microcontroller we
      // need to wait a short while so that it can respond.
      //
      // 1ms seems to be enough but it depends on what workload it has
      usleep(10000);

      char buf[1];
      if (read(file, buf, 1) == 1) {
    int temp = (int) buf[0];

    printf("Received %d\n", temp);
      }
    }

    // Now wait else you could crash the arduino by sending requests too fast
    usleep(10000);
  }
  */
  close(file);
  return (EXIT_SUCCESS);
}

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

#define NAXIS 6
#define NBUTTONS 8

struct js_event {
  __u32 time;     /* event timestamp in milliseconds */
  __s16 value;    /* value */
  __u8 type;      /* event type */
  __u8 number;    /* axis/button number */
};

struct js_state {
  __s16 axis[NAXIS];
  __u8 button[NBUTTONS];
};

#define JS_EVENT_BUTTON         0x01    /* button pressed/released */
#define JS_EVENT_AXIS           0x02    /* joystick moved */
#define JS_EVENT_INIT           0x80    /* initial state of device */

int jsfd = 0;

void joystickInit() {
  jsfd = 0;
}

void joystickState(js_state *js) {
  static js_state state;
  static bool initialized = false;
  if(!initialized) {
    memset(&state, sizeof(js_state), 0);
    initialized = true;
  }

  if(jsfd <= 0) {
    // handle device reconnect
    jsfd = open ("/dev/input/js0", O_RDONLY);
    if(jsfd <= 0) return;
  }

  pollfd pfd;
  pfd.fd = jsfd;
  pfd.events = POLLIN;
  while(poll(&pfd, 1, 0) == 1) {
    js_event event;
    if(read(jsfd, &event, sizeof(struct js_event)) != sizeof(struct js_event)) {
      // handle device disconnect
      close(jsfd);
      jsfd = 0;
      return;
    }
    event.type &= ~JS_EVENT_INIT;
    if(event.type == JS_EVENT_BUTTON && event.number < NBUTTONS) {
      state.button[event.number] = event.value;
    } else if(event.type == JS_EVENT_AXIS && event.number < NAXIS) {
      state.axis[event.number] = event.value;
    }
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
    js_state state;
    joystickState(&state);

    double speed_value = ((double)state.axis[1]) * (30.0 / 32767.0);
    double angle_value = ((double)state.axis[0]) * (20.0 / 32767.0);

    int16_t speed_ival = (int16_t)speed_value;
    int16_t angle_ival = (int16_t)angle_value;
    //printf("speed = %f, %d  angle = %f, %d\n", speed_value, speed_ival, angle_value, angle_ival);

    Message _msg;
    uint8_t* msg = (uint8_t*)&_msg;
    messageSignedInit(&_msg, COMMAND_SET_MOTION, speed_ival, angle_ival, id++);

    int nwrote = 0;
    while(nwrote != sizeof(Message)) {
      int wrote = write(file, &msg[nwrote], sizeof(Message) - nwrote);
      if(wrote != sizeof(Message)) printf("wrote = %d\n", wrote);
      if(wrote > 0) {
        nwrote += wrote;
      }
      if(nwrote < sizeof(Message)) usleep(10000);
    }

   // usleep(10000);
    Message _reply;
    uint8_t* reply = (uint8_t*)&_reply;


    const uint NRETRIES = 20;
    for(uint ii = 0; ii < NRETRIES; ++ii) {
      int nread = 0;
      while(nread != sizeof(Message)) {
        int justRead = read(file, &reply[nread], sizeof(Message) - nread);
        if(justRead != sizeof(Message)) printf("read = %d\n", justRead);
        if(justRead > 0) {
          nread += justRead;
        }
        if(nread < sizeof(Message)) usleep(10000);
      }
      if(_reply.type == COMMAND_SET_MOTION && _reply.id == _msg.id) {
        break;
      }
      if(ii == (NRETRIES-1)) {
        printf("read %d bytes, type is %d, payload is %d id: %d vs %d\n", nread, _reply.type, messagePayload(&_reply), _reply.id, _msg.id);
      } else {
        // give the board time to handle the message
        usleep(1000);
      }
    }
    //usleep(100000);
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

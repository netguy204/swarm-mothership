#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <asm/types.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>

#include "protocol.h"
#include "joystick.h"

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
  char* err;
  int ADDRESS = strtol(argv[2], &err, 10);
  if(!*argv[2] || *err) {
    fprintf(stderr, "usage: %s <i2c-dev> <i2c-slave-number>\n", argv[0]);
    fprintf(stderr, "       i2c-slave-number must be a number\n");
    exit(1);
  }


  printf("I2C: Connecting\n");
  int file;

  if ((file = open(devName, O_RDWR)) < 0) {
    fprintf(stderr, "I2C: Failed to access %s: %s\n", devName, strerror(errno));
    exit(1);
  }

  printf("I2C: acquiring bus to %#x\n", ADDRESS);

  if (ioctl(file, I2C_SLAVE, ADDRESS) < 0) {
    fprintf(stderr, "I2C: Failed to acquire bus access/talk to slave %#x: %s\n",
        ADDRESS, strerror(errno));
                close(file);
    exit(1);
  }

  joystickInit();
  uint8_t id = 0;

  while(true) {
    js_state state;
    joystickState(&state);

    double speed_value = (static_cast<double>(state.axis[1])) * (30.0 / 32767.0);
    double angle_value = (static_cast<double>(state.axis[0])) * (20.0 / 32767.0);

    // deadzones
    if(speed_value > -3 && speed_value < 3) {
      speed_value = 0;
    }
    if(angle_value > -2 && angle_value < 2) {
      angle_value = 0;
    }

    int16_t speed_ival = static_cast<int16_t>(speed_value);
    int16_t angle_ival = static_cast<int16_t>(angle_value);
    //printf("speed = %f, %d  angle = %f, %d\n", speed_value, speed_ival, angle_value, angle_ival);

    Message _msg;
    uint8_t* msg = reinterpret_cast<uint8_t*>(&_msg);
    messageSignedInit(&_msg, COMMAND_SET_MOTION, speed_ival, angle_ival, id++);

    ssize_t nwrote = 0;
    while(nwrote != sizeof(Message)) {
      ssize_t wrote = write(file, &msg[nwrote], sizeof(Message) - nwrote);
      if(wrote == -1) {
        fprintf(stderr, "write failed: (%d) %s\n", errno, strerror(errno));
      }
      if(wrote != sizeof(Message)) printf("wrote = %ld\n", static_cast<long>(wrote));
      if(wrote > 0) {
        nwrote += wrote;
      }
      if(static_cast<size_t>(nwrote) < sizeof(Message)) usleep(COMMAND_DURATION_US / 4);
    }

    Message _reply;
    uint8_t* reply = reinterpret_cast<uint8_t*>(&_reply);

    const uint NRETRIES = 20;
    for(uint ii = 0; ii < NRETRIES; ++ii) {
      ssize_t nread = 0;
      while(nread != sizeof(Message)) {
        ssize_t justRead = read(file, &reply[nread], sizeof(Message) - nread);
        if(justRead == -1) {
          fprintf(stderr, "read failed: (%d) %s\n", errno, strerror(errno));
        }
        if(justRead != sizeof(Message)) printf("read = %ld\n", static_cast<long>(justRead));
        if(justRead > 0) {
          nread += justRead;
        }
        if(static_cast<size_t>(nread) < sizeof(Message)) usleep(COMMAND_DURATION_US / 4);
      }
      if(_reply.type == COMMAND_SET_MOTION && _reply.id == _msg.id) {
        break;
      }
      if(ii == (NRETRIES-1)) {
        printf("read %ld bytes, type is %d, payload is %d id: %d vs %d\n", static_cast<long>(nread), _reply.type, messagePayload(&_reply), _reply.id, _msg.id);
      } else {
        // give the board time to handle the message, but still get
        // the next one in to allow chaining
        usleep(COMMAND_DURATION_US / 2);
      }
    }
    //usleep(100000);
  }

  close(file);
  return (EXIT_SUCCESS);
}

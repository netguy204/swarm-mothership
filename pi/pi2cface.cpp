#include <algorithm>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "joystick.h"
#include "pfsm.h"
#include "protocol.h"
#include "systemtime.h"
#include "wsfsm.h"


int main(int argc, char** argv) {

  if (argc != 3) {
    fprintf(stderr, "usage: %s i2c-dev i2c-slave-number\n", argv[0]);
    exit(1);
  }

  const char* bus = argv[1];
  char* err;
  int dev = strtol(argv[2], &err, 10);
  if(!*argv[2] || *err) {
    fprintf(stderr, "usage: %s <i2c-dev> <i2c-slave-number>\n", argv[0]);
    fprintf(stderr, "       i2c-slave-number must be a number\n");
    exit(1);
  }

  WebServiceFSM wsfsm{"http://localhost:8080/"};

  RealProtocolFSM pfsm;
  pfsm.init(bus, dev);

  joystickInit();
  uint8_t id = 0;

  // ProtocolState old_state = pfsm.state;

  while(true) {
    js_state state;
    joystickState(&state);

    wsfsm.update();
    pfsm.update();

    /* DEBUGGING
    if(pfsm.state != old_state) {
      fprintf(stderr, "PFSM: %s => %s\n", ProtocolStateStr[old_state], ProtocolStateStr[pfsm.state]);
      old_state = pfsm.state;
    }
    */
      Message msg;

    // can we send another message?
    if(pfsm.state == IDLE) {
      double speed_value = 0.0;
      double angle_value = 0.0;

      if(Time() - lastJoystickUpdate() < TimeLength::inSeconds(1)) {

        speed_value = (static_cast<double>(state.axis[1])) * (33.0 / 32767.0);
        angle_value = (static_cast<double>(state.axis[0])) * (-30.0 / 32767.0);

        // deadzones
        if(speed_value > -5 && speed_value < 5) {
          speed_value = 0;
        }
        if(angle_value > -2 && angle_value < 2) {
          angle_value = 0;
        }
      } else if (wsfsm.command_available) {
        speed_value = wsfsm.command.speed * 33.0;
        angle_value = std::max(-30.0, std::min(30.0, wsfsm.command.angle*-1.0));
        wsfsm.command_completed = true;
      }
      int8_t speed_ival = static_cast<int8_t>(speed_value);
      int8_t angle_ival = static_cast<int8_t>(angle_value);
      //printf("speed = %f, %d  angle = %f, %d\n", speed_value, speed_ival, angle_value, angle_ival);

      messageSignedInit(&msg, COMMAND_SET_MOTION, speed_ival, angle_ival, id++);

      pfsm.send(&msg);

    }


    if(pfsm.state == SENDING_FAILED || pfsm.state == ACKING_FAILED) {
      // don't care for now
      fprintf(stderr, "ignorning error\n");
      pfsm.clearError();
      messageSignedInit(&msg, COMMAND_SET_MOTION, speed_ival, angle_ival, id++);
      pfsm.send(&msg);
      // webservice: will be implemented in wsfsm.cpp later
    }

    if(pfsm.state == ACK_COMPLETE) {
      pfsm.acknowledgeAck();
    }

    // sleep off any delay the protocol is waiting for because we have
    // nothing better to do right now - imp. b/c running on battery
    TimeLength delay = pfsm.delayRemaining();
    if(delay > TimeLength::inSeconds(0)) {
      usleep(delay.microseconds());
    }
  }

  return (EXIT_SUCCESS);
}

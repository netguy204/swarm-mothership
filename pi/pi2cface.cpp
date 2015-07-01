#include <stdio.h>
#include <stdlib.h>

#include "pfsm.h"
#include "protocol.h"
#include "joystick.h"
#include "systemtime.h"


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

  RealProtocolFSM pfsm;
  pfsm.init(bus, dev);

  joystickInit();
  uint8_t id = 0;

  // ProtocolState old_state = pfsm.state;

  while(true) {
    js_state state;
    joystickState(&state);

    pfsm.update();

    /* DEBUGGING
    if(pfsm.state != old_state) {
      fprintf(stderr, "PFSM: %s => %s\n", ProtocolStateStr[old_state], ProtocolStateStr[pfsm.state]);
      old_state = pfsm.state;
    }
    */

    // can we send another message?
    if(pfsm.state == IDLE) {
      if(Time() - lastJoystickUpdate() < TimeLength::inSeconds(10)) {


	double speed_value = (static_cast<double>(state.axis[1])) * (63.0 / 32767.0);
	double angle_value = (static_cast<double>(state.axis[0])) * (30.0 / 32767.0);

	// deadzones
	if(speed_value > -5 && speed_value < 5) {
	  speed_value = 0;
	}
	if(angle_value > -2 && angle_value < 2) {
	  angle_value = 0;
	}

	int8_t speed_ival = static_cast<int8_t>(speed_value);
	int8_t angle_ival = static_cast<int8_t>(angle_value);
	//printf("speed = %f, %d  angle = %f, %d\n", speed_value, speed_ival, angle_value, angle_ival);

	Message msg;
	messageSignedInit(&msg, COMMAND_SET_MOTION, speed_ival, angle_ival, id++);

	pfsm.send(&msg);
      } else {

	// ask the webservice

	// format the command

	// pfsm.send(command)

	// remember what we sent last
      }
    }

    if(pfsm.state == SENDING_FAILED || pfsm.state == ACKING_FAILED) {
      // don't care for now
      fprintf(stderr, "ignorning error\n");
      pfsm.clearError();

      // webservice: resend what we sent last
    }

    if(pfsm.state == ACK_COMPLETE) {
      // TODO: tell the webservice that we did what was asked
      pfsm.acknowledgeAck();

      // webservice: tell the service we finished the command
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

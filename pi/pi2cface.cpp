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
#include "systemtime.h"

#define PROTOCOL_STATE(m) \
  m(DISCONNECTED),           \
  m(DISCONNECTED_COOLDOWN),  \
  m(IDLE),                   \
  m(SENDING),                \
  m(SENDING_FAILED),         \
  m(ACKING),                 \
  m(ACKING_FAILED),          \
  m(ACK_COMPLETE),           \
  m(STATE_MAX)

#define CREATE_ENUM(v) v
#define CREATE_STRING(v) #v

enum ProtocolState {
  PROTOCOL_STATE(CREATE_ENUM)
};

const char* ProtocolStateStr[] = {
  PROTOCOL_STATE(CREATE_STRING)
};

static TimeLength DISCONNECTED_COOLDOWN_TIME = TimeLength::inSeconds(10);
static TimeLength WRITE_AGAIN_COOLDOWN_TIME = TimeLength::inMilliseconds(1);
static TimeLength READ_AGAIN_COOLDOWN_TIME = TimeLength::inMilliseconds(1);
static TimeLength ACK_AGAIN_COOLDOWN_TIME = TimeLength::inMilliseconds(COMMAND_DURATION_MS/2);
static const uint8_t MAX_ACK_ATTEMPTS = 20;

struct PFSM {
  Message last_sent;
  Message last_received;

  Message sending;
  Message receiving;
  Message to_send;

  Time state_start;
  TimeLength state_duration;

  ProtocolState state;

  const char* bus;
  int fp;
  uint8_t dev;
  uint8_t sending_offset;
  uint8_t receiving_offset;
  uint8_t ack_attempts;

  uint8_t outbound_message_waiting : 1;
  uint8_t inbound_message_waiting : 1;
  uint8_t ack_acknowledged : 1;
  uint8_t failure_acknowledged : 1;
};

TimeLength protocolDelayRemaining(PFSM* pfsm) {
  return pfsm->state_duration - (Time() - pfsm->state_start);
}

bool protocolDelayExpired(PFSM* pfsm) {
  return (Time() - pfsm->state_start) > pfsm->state_duration;
}

void protocolSetDelay(PFSM* pfsm, const TimeLength& duration) {
  pfsm->state_start = Time();
  pfsm->state_duration = duration;
}

void protocolInit(PFSM* pfsm, const char* bus, uint8_t dev) {
  memset(pfsm, sizeof(PFSM), 0);

  pfsm->state = DISCONNECTED;
  pfsm->bus = bus;
  pfsm->dev = dev;
}

void protocolUpdate(PFSM* pfsm) {
  if(pfsm->state == DISCONNECTED_COOLDOWN && protocolDelayExpired(pfsm)) {
    pfsm->state = DISCONNECTED;
  }

  if(pfsm->state == DISCONNECTED) {
    // attempt connection
    fprintf(stderr, "I2C: Connecting\n");
    if((pfsm->fp = open(pfsm->bus, O_RDWR)) < 0) {
      fprintf(stderr, "I2C: Failed to access %s: %s\n", pfsm->bus, strerror(errno));
      pfsm->state = DISCONNECTED_COOLDOWN;
      protocolSetDelay(pfsm, DISCONNECTED_COOLDOWN_TIME);

      return;
    }

    fprintf(stderr, "I2C: acquiring bus to %#x\n", pfsm->dev);

    if (ioctl(pfsm->fp, I2C_SLAVE, pfsm->dev) < 0) {
      fprintf(stderr, "I2C: Failed to acquire bus access/talk to slave %#x: %s\n",
              pfsm->dev, strerror(errno));
      close(pfsm->fp);
      pfsm->state = DISCONNECTED_COOLDOWN;
      protocolSetDelay(pfsm, DISCONNECTED_COOLDOWN_TIME);

      return;
    }

    fprintf(stderr, "I2C: Connection successful\n");
    pfsm->state = IDLE;
  }

  if(pfsm->state == IDLE && pfsm->outbound_message_waiting) {
    pfsm->state = SENDING;
    pfsm->sending = pfsm->to_send;
    pfsm->sending_offset = 0;
    pfsm->outbound_message_waiting = false;
  }

  if(pfsm->state == SENDING && protocolDelayExpired(pfsm)) {
    uint8_t* msg = (uint8_t*)&pfsm->sending;
    ssize_t wrote = write(pfsm->fp, (const void*)&msg[pfsm->sending_offset], sizeof(Message) - pfsm->sending_offset);
    if(wrote == -1) {
      fprintf(stderr, "I2C: write failed: (%d) %s\n", errno, strerror(errno));
      if(errno == EAGAIN || errno == EWOULDBLOCK) {
        protocolSetDelay(pfsm, WRITE_AGAIN_COOLDOWN_TIME);
      } else if(errno == EBADF) {
        pfsm->state = DISCONNECTED;
      } else {
        // switch to a fail state so the user can decide what happens next
        pfsm->state = SENDING_FAILED;
        pfsm->failure_acknowledged = false;
      }
      return;
    }
    pfsm->sending_offset += wrote;
    if(pfsm->sending_offset == sizeof(Message)) {
      pfsm->state = ACKING;
      pfsm->last_sent = pfsm->sending;
      pfsm->ack_attempts = 0;
      pfsm->receiving_offset = 0;
    }
  }

  if(pfsm->state == ACKING && protocolDelayExpired(pfsm)) {
    uint8_t* msg = (uint8_t*)&pfsm->receiving;
    ssize_t nread = read(pfsm->fp, (void*)&msg[pfsm->receiving_offset], sizeof(Message) - pfsm->receiving_offset);
    if(nread == -1) {
      if(errno == EAGAIN || errno == EWOULDBLOCK) {
        protocolSetDelay(pfsm, READ_AGAIN_COOLDOWN_TIME);
      } else if(errno == EBADF) {
        pfsm->state = DISCONNECTED;
      } else {
        fprintf(stderr, "I2C: read failed: (%d) %s\n", errno, strerror(errno));
        pfsm->state = ACKING_FAILED;
        pfsm->failure_acknowledged = false;
      }
      return;
    }
    pfsm->receiving_offset += nread;
    if(pfsm->receiving_offset == sizeof(Message)) {
      pfsm->last_received = pfsm->receiving;

      if(pfsm->last_received.type == pfsm->last_sent.type && pfsm->last_received.id == pfsm->last_sent.id) {
        pfsm->state = ACK_COMPLETE;
        pfsm->ack_acknowledged = false;
      } else {
        pfsm->ack_attempts++;
        pfsm->receiving_offset = 0;

        /*
        fprintf(stderr, "Attempt %d: type %d vs %d, id %d vs %d\n", pfsm->ack_attempts,
                pfsm->last_received.type, pfsm->last_sent.type,
                pfsm->last_received.id, pfsm->last_sent.id);
        */
        if(pfsm->ack_attempts == MAX_ACK_ATTEMPTS) {
          pfsm->state = ACKING_FAILED;
          pfsm->failure_acknowledged = false;
        } else {
          //fprintf(stderr, "read succeeded but wrong result, retry\n");
          protocolSetDelay(pfsm, ACK_AGAIN_COOLDOWN_TIME);
        }
      }
    }
  }

  if(pfsm->state == ACK_COMPLETE && pfsm->ack_acknowledged) {
    pfsm->state = IDLE;
  }

  if(pfsm->state == ACKING_FAILED && pfsm->failure_acknowledged) {
    pfsm->state = IDLE;
  }

  if(pfsm->state == SENDING_FAILED && pfsm->failure_acknowledged) {
    pfsm->state = IDLE;
  }
}


bool protocolSend(PFSM* pfsm, Message* message) {
  if(pfsm->outbound_message_waiting) return false;

  pfsm->to_send = *message;
  pfsm->outbound_message_waiting = true;
  return true;
}

bool protocolAcknowledgeAck(PFSM* pfsm) {
  if(pfsm->ack_acknowledged) return false;

  pfsm->ack_acknowledged = true;
  return true;
}

bool protocolClearError(PFSM* pfsm) {
  if(pfsm->failure_acknowledged) return false;

  pfsm->failure_acknowledged = true;
  return true;
}

bool protocolClose(PFSM* pfsm) {
  if(pfsm->fp == 0) return false;
  close(pfsm->fp);
  pfsm->state = DISCONNECTED;
  return true;
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

  const char* bus = argv[1];
  char* err;
  int dev = strtol(argv[2], &err, 10);
  if(!*argv[2] || *err) {
    fprintf(stderr, "usage: %s <i2c-dev> <i2c-slave-number>\n", argv[0]);
    fprintf(stderr, "       i2c-slave-number must be a number\n");
    exit(1);
  }

  PFSM pfsm;
  protocolInit(&pfsm, bus, dev);
  joystickInit();
  uint8_t id = 0;

  // ProtocolState old_state = pfsm.state;

  while(true) {
    js_state state;
    joystickState(&state);

    protocolUpdate(&pfsm);

    /* DEBUGGING
    if(pfsm.state != old_state) {
      fprintf(stderr, "PFSM: %s => %s\n", ProtocolStateStr[old_state], ProtocolStateStr[pfsm.state]);
      old_state = pfsm.state;
    }
    */

    // can we send another message?
    if(pfsm.state == IDLE) {
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

      protocolSend(&pfsm, &msg);
    }

    if(pfsm.state == SENDING_FAILED || pfsm.state == ACKING_FAILED) {
      // don't care for now
      fprintf(stderr, "ignorning error\n");
      protocolClearError(&pfsm);
    }

    if(pfsm.state == ACK_COMPLETE) {
      // TODO: tell the webservice that we did what was asked
      protocolAcknowledgeAck(&pfsm);
    }

    // sleep off any delay the protocol is waiting for because we have
    // nothing better to do right now
    TimeLength delay = protocolDelayRemaining(&pfsm);
    if(delay > TimeLength::inSeconds(0)) {
      usleep(delay.microseconds());
    }
  }

  return (EXIT_SUCCESS);
}

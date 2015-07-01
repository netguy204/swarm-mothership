#include "pfsm.h"
#include <asm/types.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

const char* ProtocolStateStr[] = {
  PROTOCOL_STATE(CREATE_STRING)
};

TimeLength ProtocolFSM::DISCONNECTED_COOLDOWN_TIME = TimeLength::inSeconds(10);
TimeLength ProtocolFSM::WRITE_AGAIN_COOLDOWN_TIME = TimeLength::inMilliseconds(1);
TimeLength ProtocolFSM::READ_AGAIN_COOLDOWN_TIME = TimeLength::inMilliseconds(1);
TimeLength ProtocolFSM::ACK_AGAIN_COOLDOWN_TIME = TimeLength::inMilliseconds(COMMAND_DURATION_MS/2);
const uint8_t ProtocolFSM::MAX_ACK_ATTEMPTS = 20;

TimeLength ProtocolFSM::delayRemaining() {
  return state_duration - (Time() - state_start);
}

bool ProtocolFSM::delayExpired() {
  return (Time() - state_start) > state_duration;
}

void ProtocolFSM::setDelay(const TimeLength& duration) {
  state_start = Time();
  state_duration = duration;
}







void RealProtocolFSM::init(const char* _bus, uint8_t _dev) {
  state = DISCONNECTED;
  bus = _bus;
  dev = _dev;
  outbound_message_waiting = false;
  ack_acknowledged = false;
  failure_acknowledged = false;
}

void RealProtocolFSM::update() {
  if(state == DISCONNECTED_COOLDOWN && delayExpired()) {
    state = DISCONNECTED;
  }

  if(state == DISCONNECTED) {
    /*
     * Arduino is a pure I2C slave. This client writes commands to it and
     * reads the response after enough time has passed.
     */

    // attempt connection
    fprintf(stderr, "I2C: Connecting\n");
    if((fp = open(bus, O_RDWR)) < 0) {
      fprintf(stderr, "I2C: Failed to access %s: %s\n", bus, strerror(errno));
      state = DISCONNECTED_COOLDOWN;
      setDelay(DISCONNECTED_COOLDOWN_TIME);

      return;
    }

    fprintf(stderr, "I2C: acquiring bus to %#x\n", dev);

    if (ioctl(fp, I2C_SLAVE, dev) < 0) {
      fprintf(stderr, "I2C: Failed to acquire bus access/talk to slave %#x: %s\n",
              dev, strerror(errno));
      ::close(fp);
      state = DISCONNECTED_COOLDOWN;
      setDelay(DISCONNECTED_COOLDOWN_TIME);

      return;
    }

    fprintf(stderr, "I2C: Connection successful\n");
    state = IDLE;
  }

  if(state == IDLE && outbound_message_waiting) {
    state = SENDING;
    sending = to_send;
    sending_offset = 0;
    outbound_message_waiting = false;
  }

  if(state == SENDING && delayExpired()) {
    uint8_t* msg = (uint8_t*)&sending;
    ssize_t wrote = write(fp, (const void*)&msg[sending_offset], sizeof(Message) - sending_offset);
    if(wrote == -1) {
      fprintf(stderr, "I2C: write failed: (%d) %s\n", errno, strerror(errno));
      if(errno == EAGAIN || errno == EWOULDBLOCK) {
        setDelay(WRITE_AGAIN_COOLDOWN_TIME);
      } else if(errno == EBADF) {
        state = DISCONNECTED;
      } else {
        // switch to a fail state so the user can decide what happens next
        state = SENDING_FAILED;
        failure_acknowledged = false;
      }
      return;
    }
    sending_offset += wrote;
    if(sending_offset == sizeof(Message)) {
      state = ACKING;
      last_sent = sending;
      ack_attempts = 0;
      receiving_offset = 0;
    }
  }

  if(state == ACKING && delayExpired()) {
    uint8_t* msg = (uint8_t*)&receiving;
    ssize_t nread = read(fp, (void*)&msg[receiving_offset], sizeof(Message) - receiving_offset);
    if(nread == -1) {
      if(errno == EAGAIN || errno == EWOULDBLOCK) {
        setDelay(READ_AGAIN_COOLDOWN_TIME);
      } else if(errno == EBADF) {
        state = DISCONNECTED;
      } else {
        fprintf(stderr, "I2C: read failed: (%d) %s\n", errno, strerror(errno));
        state = ACKING_FAILED;
        failure_acknowledged = false;
      }
      return;
    }
    receiving_offset += nread;
    if(receiving_offset == sizeof(Message)) {
      last_received = receiving;

      if(last_received.type == last_sent.type && last_received.id == last_sent.id) {
        state = ACK_COMPLETE;
        ack_acknowledged = false;
      } else {
        ack_attempts++;
        receiving_offset = 0;

        /*
          fprintf(stderr, "Attempt %d: type %d vs %d, id %d vs %d\n", ack_attempts,
          last_received.type, last_sent.type,
          last_received.id, last_sent.id);
        */
        if(ack_attempts == MAX_ACK_ATTEMPTS) {
          state = ACKING_FAILED;
          failure_acknowledged = false;
        } else {
          //fprintf(stderr, "read succeeded but wrong result, retry\n");
          setDelay(ACK_AGAIN_COOLDOWN_TIME);
        }
      }
    }
  }

  if(state == ACK_COMPLETE && ack_acknowledged) {
    state = IDLE;
  }

  if(state == ACKING_FAILED && failure_acknowledged) {
    state = IDLE;
  }

  if(state == SENDING_FAILED && failure_acknowledged) {
    state = IDLE;
  }
}

bool RealProtocolFSM::send(const Message* message) {
  if(outbound_message_waiting) return false;

  to_send = *message;
  outbound_message_waiting = true;
  return true;
}

bool RealProtocolFSM::acknowledgeAck() {
  if(ack_acknowledged) return false;

  ack_acknowledged = true;
  return true;
}

bool RealProtocolFSM::clearError() {
  if(failure_acknowledged) return false;

  failure_acknowledged = true;
  return true;
}

bool RealProtocolFSM::close() {
  if(fp == 0) return false;
  ::close(fp);
  state = DISCONNECTED;
  return true;
}

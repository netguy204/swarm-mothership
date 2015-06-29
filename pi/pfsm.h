#ifndef PFSM_H
#define PFSM_H

#include "systemtime.h"
#include "protocol.h"
#include <stdint.h>

/**
 * States. Declared in this clever way so that we can easily generate
 * the parallel list of strings that go with them. See:
 * http://www.50ply.com/blog/2013/04/19/eliminate-parallel-lists-with-higher-order-macros/
 */
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

extern const char* ProtocolStateStr[];

/**
 * Base class for the PI side of the protocol finite state machine.
 * Portions of this are pure virtual so that the interface with the
 * alamode can be emulated or real.
 */
class ProtocolFSM {
 private:
  Time state_start;
  TimeLength state_duration;

 public:
  static TimeLength DISCONNECTED_COOLDOWN_TIME;
  static TimeLength WRITE_AGAIN_COOLDOWN_TIME;
  static TimeLength READ_AGAIN_COOLDOWN_TIME;
  static TimeLength ACK_AGAIN_COOLDOWN_TIME;
  static const uint8_t MAX_ACK_ATTEMPTS;


  ProtocolState state;

  TimeLength delayRemaining();
  bool delayExpired();
  void setDelay(const TimeLength& duration);

  virtual void init(const char* bus, uint8_t dev) = 0;
  virtual void update() = 0;
  virtual bool send(const Message* msg) = 0;
  virtual bool acknowledgeAck() = 0;
  virtual bool clearError() = 0;
  virtual bool close() = 0;
};


class RealProtocolFSM : public ProtocolFSM {
 private:
  Message last_sent;
  Message last_received;

  Message sending;
  Message receiving;
  Message to_send;

  const char* bus;
  int fp;
  uint8_t dev;
  uint8_t sending_offset;
  uint8_t receiving_offset;
  uint8_t ack_attempts;

  uint8_t outbound_message_waiting : 1;
  uint8_t ack_acknowledged : 1;
  uint8_t failure_acknowledged : 1;

 public:

  virtual void init(const char* _bus, uint8_t _dev);
  virtual void update();
  virtual bool send(const Message* message);
  virtual bool acknowledgeAck();
  virtual bool clearError();
  virtual bool close();
};




#endif

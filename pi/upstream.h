#ifndef UPFSM_H
#define UPFSM_H

#include "systemtime.h"
#include "protocol.h"
#include <stdint.h>

/**
 * States. Declared in this clever way so that we can easily generate
 * the parallel list of strings that go with them. See:
 * http://www.50ply.com/blog/2013/04/19/eliminate-parallel-lists-with-higher-order-macros/
 */
#define UPSTREAM_STATE(m) \
  m(DISCONNECTED),           \
  m(CONNECTED),              \
  m(DISCONNECTED_COOLDOWN),  \
  m(IDLE),                   \
  m(SENDING),                \
  m(SENDING_FAILED),         \
  m(ACKING),                 \
  m(ACKING_FAILED),          \
  m(ACK_COMPLETE),           \
  m(STATE_MAX)

#define UCREATE_ENUM(v) v
#define UCREATE_STRING(v) #v

enum class UpstreamState {
  UPSTREAM_STATE(UCREATE_ENUM)
};

extern const char* UpstreamStateStr[];

/**
 * Abstract class for the PI side of the Upstream finite state machine.
 * Portions of this are pure virtual so that the interface with the
 * webservice can be emulated or real.
 */
class UpstreamFSM {
 private:
  Time state_start;
  TimeLength state_duration;

 public:
  static const TimeLength DISCONNECTED_COOLDOWN_TIME;
  static const TimeLength WRITE_AGAIN_COOLDOWN_TIME;
  static const TimeLength READ_AGAIN_COOLDOWN_TIME;
  static const TimeLength ACK_AGAIN_COOLDOWN_TIME;
  static const uint8_t MAX_ACK_ATTEMPTS;


  bool delayExpired();
  void setDelay(const TimeLength& duration);
  TimeLength delayRemaining();

  virtual bool send(const Message* msg) = 0;

  UpstreamState state;


  virtual void update() = 0;
  virtual bool acknowledgeAck() = 0;
  virtual bool clearError() = 0;
  virtual bool close() = 0;
};

#endif

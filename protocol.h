#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

enum MessageType {
  COMMAND_NOOP,
  COMMAND_SET_SPEED,
  COMMAND_SET_SERVO,
  
  REQUEST_TEMPERATURE,
  
  REPORT_CURRENT_SPEED,
  REPORT_COMMAND_COMPLETE,
  REPORT_USER_OVERRIDE,
  REPORT_MOTOR_ERROR,
  REPORT_NOOP,
  MESSAGE_MAX
};

struct Message {
  uint8_t type;
  uint8_t payload_low;
  uint8_t payload_high;
  uint8_t id;
  uint8_t checksum;
};

uint8_t messageChecksum(Message* msg);
void messageInit(Message* msg, MessageType type, uint16_t value);
uint16_t messagePayload(Message* msg);

#endif


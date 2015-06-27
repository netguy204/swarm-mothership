#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

enum MessageType {
  COMMAND_NOOP = 0x80,
  COMMAND_SET_MOTION,

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
};

uint8_t messageChecksum(volatile Message* msg);
void messageInit(volatile Message* msg, MessageType type, uint16_t value, uint8_t id);
void messageInit(volatile Message* msg, MessageType type, uint8_t low, uint8_t high, uint8_t id);
uint16_t messagePayload(volatile Message* msg);

void messageSignedInit(volatile Message* msg, MessageType type, int16_t value, uint8_t id);
void messageSignedInit(volatile Message* msg, MessageType type, int8_t low, int8_t high, uint8_t id);
int16_t messageSignedPayload(volatile Message* msg);

int8_t messageSignedPayloadLow(volatile Message* msg);
int8_t messageSignedPayloadHigh(volatile Message* msg);

#endif

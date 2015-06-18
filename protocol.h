#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>
#include <Arduino.h>
enum MessageType {
  COMMAND_SET_SPEED,
  REPORT_CURRENT_SPEED,
  COMMAND_SYNC_STREAM,
  COMMAND_MAX
};

struct Message {
  uint8_t type;
  uint8_t payload_low;
  uint8_t payload_high;
  uint8_t checksum;
};

void sendMessage(Stream& stream, MessageType type, uint16_t payload);

bool hasMessage(Message* message, Stream& stream);

#endif


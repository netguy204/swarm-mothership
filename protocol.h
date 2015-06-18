#ifndef PROTOCOL_H
#define PROTOCOL_H

#include <stdint.h>

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

#ifdef ARDUINO
#include <Arduino.h>
void sendMessage(Stream& stream, MessageType type, uint16_t payload);
bool hasMessage(Message* message, Stream& stream);
#else
void sendMessage(int stream, MessageType type, uint16_t payload);
bool hasMessage(Message* message, int stream);
#endif
#endif


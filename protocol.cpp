#include "protocol.h"


uint8_t messageChecksum(Message* msg) {
  return (msg->type ^ msg->payload_low ^ msg->payload_high);
}

void messageInit(Message* msg, MessageType type, uint16_t value) {
  msg->type = type;
  msg->payload_low = value & 0xFF;
  msg->payload_high = (value >> 8) & 0xFF;
}

uint16_t messagePayload(Message* msg) {
  uint16_t result = msg->payload_high;
  result <<= 8;
  return result | msg->payload_low;
}

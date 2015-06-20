#include "protocol.h"


uint8_t messageChecksum(volatile Message* msg) {
  return (msg->type ^ msg->payload_low ^ msg->payload_high);
}

void messageInit(volatile Message* msg, MessageType type, uint16_t value, uint8_t id) {
  msg->type = type;
  msg->payload_low = value & 0x7F;
  msg->payload_high = (value >> 7) & 0x7F;
  msg->id = id & 0x7F;
}

uint16_t messagePayload(volatile Message* msg) {
  uint16_t result = msg->payload_high;
  result <<= 7;
  return result | msg->payload_low;
}

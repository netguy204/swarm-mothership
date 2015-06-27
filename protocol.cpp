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

void messageInit(volatile Message* msg, MessageType type, uint8_t low, uint8_t high, uint8_t id) {
  msg->type = type;
  msg->payload_low = low & 0x7F;
  msg->payload_high = high & 0x7F;
  msg->id = id & 0x7F;
}

uint16_t messagePayload(volatile Message* msg) {
  uint16_t result = msg->payload_high;
  result <<= 7;
  return result | msg->payload_low;
}

void messageSignedInit(volatile Message* msg, MessageType type, int16_t value, uint8_t id) {
  msg->type = type;
  msg->payload_low = value & 0x7F;
  msg->payload_high = (value >> 7) & 0x7F;
  msg->id = id & 0x7F;
}

int16_t messageSignedPayload(volatile Message* msg) {
  uint16_t uresult = messagePayload(msg);
  int16_t result = (uresult << 2);
  return result >> 2;
}

int8_t messageSignedPayloadLow(volatile Message* msg) {
  int16_t result = (msg->payload_low << 1);
  return result >> 1;
}

int8_t messageSignedPayloadHigh(volatile Message* msg) {
  int16_t result = (msg->payload_high << 1);
  return result >> 1;
}

#include "protocol.h"

#ifndef ARDUINO
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#endif

uint8_t messageChecksum(volatile Message* msg) {
  return (msg->type ^ msg->payload_low ^ msg->payload_high);
}

void messageInit(volatile Message* msg, MessageType type, uint16_t value, uint8_t id) {
  msg->type = type;
  msg->payload_low = value & 0x9F;
  msg->payload_high = (value >> 7) & 0x9F;
  msg->id = id & 0x9F;
}

uint16_t messagePayload(volatile Message* msg) {
  uint16_t result = msg->payload_high;
  result <<= 7;
  return result | msg->payload_low;
}

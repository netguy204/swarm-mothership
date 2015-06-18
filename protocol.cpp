#include "protocol.h"

#ifndef ARDUINO
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#endif

uint8_t messageChecksum(Message* msg) {
  return (msg->type + msg->payload_low + msg->payload_high);
}

#ifdef ARDUINO
void sendMessage(Stream& stream, MessageType type, uint16_t payload) {
  #define WRITEBYTES(x, y, n) (x.write(y, n))
#else
void sendMessage(int stream, MessageType type, uint16_t payload) {
  #define WRITEBYTES(x, y, n)    \
  do {                           \
    write(stream, y, n);         \
  } while(0)
#endif
  Message msg;
  msg.type = type;
  msg.payload_low = payload & 0xFF;
  msg.payload_high = (payload >> 8) & 0xFF;
  msg.checksum = messageChecksum(&msg);
  WRITEBYTES(stream, (char*)&msg, sizeof(msg));
}

#ifdef ARDUINO
bool hasMessage(Message* message, Stream& stream) {
  #define READBYTE(x, y) do {     \
    int v;                        \
    while((v = x.read()) == -1);  \
    y = v;                        \
  } while(0)
#else
uint8_t tempRead;
bool hasMessage(Message* message, int stream) {
  #define READBYTE(x, y) \
  do {                   \
    while(read(stream, &tempRead, 1) != 1); \
    y = tempRead; \
  } while(0)
#endif

  char* buf = (char*)message;
  for(uint8_t ii = 0; ii < sizeof(Message); ++ii) {
    READBYTE(stream, buf[ii]);
  }
  if(messageChecksum(message) == message->checksum) return true;

  // effectively block until there's no sign of synchronization remaining in the stream
  while(true) {
    // check to see if this contains a sync request
    int8_t sync = -1;
    for(uint8_t ii = 0; ii < sizeof(Message); ++ii) {
      if(buf[ii] == COMMAND_SYNC_STREAM) {
        sync = ii;
      }
    }

    if(sync == -1) return false; // unrecoverable error. hopefully the sender syncs in the future

    // slide up to the sync
    for(uint8_t ii = sync; ii < sizeof(Message); ++ii) {
      buf[ii - sync] = buf[ii];
    }

    // begin rotating the buffer until it begins with a non-sync
    while(buf[0] == COMMAND_SYNC_STREAM) {
      for(uint8_t ii = 1; ii < sizeof(Message); ++ii) {
        buf[ii - 1] = buf[ii];
      }
      READBYTE(stream, buf[sizeof(Message) - 1]);
    }

    // do we have a valid message?
    if(messageChecksum(message) == message->checksum) return true;
  }
}

#include "protocol.h"


uint8_t messageChecksum(Message* msg) {
  return (msg->type + msg->payload_low + msg->payload_high);
}

void sendMessage(Stream& stream, MessageType type, uint16_t payload) {
  Message msg;
  msg.type = type;
  msg.payload_low = payload & 0xFF;
  msg.payload_high = (payload >> 8) & 0xFF;
  msg.checksum = messageChecksum(&msg);
  stream.write((char*)&msg, sizeof(msg));
}

bool hasMessage(Message* message, Stream& stream) {
  if(!stream.available()) return false;
  
  char* buf = (char*)message;
  for(uint8_t ii = 0; ii < sizeof(Message); ++ii) {
    buf[ii] = stream.read();
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
      buf[sizeof(Message) - 1] = stream.read();
    }
    
    // do we have a valid message?
    if(messageChecksum(message) == message->checksum) return true;
  }
}

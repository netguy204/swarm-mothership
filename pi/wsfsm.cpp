#include <asm/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "wsfsm.h"

WebServiceFSM::WebServiceFSM() {
  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();
}

WebServiceFSM::~WebServiceFSM() {
  curl_easy_cleanup(curl);
  curl_global_cleanup();
}

void WebServiceFSM::init(const char* _endpoint) {
  state = UpstreamState::DISCONNECTED;
  endpoint = _endpoint;
  outbound_message_waiting = false;
  ack_acknowledged = false;
  failure_acknowledged = false;

  curl_easy_setopt(curl, CURLOPT_URL, endpoint);
}

void WebServiceFSM::pushCmdStatus(Message status) {
  JsonObject& root = jsonBuffer.createObject();
  root["pid"] = 0; // mothership 
  root["cid"] = 1; // what should this be?
  root["status"] = "true"; // true or false

}

void WebServiceFSM::update() {
  if(state == UpstreamState::DISCONNECTED_COOLDOWN && delayExpired()) {
    state = UpstreamState::DISCONNECTED;
  }

  if(state == UpstreamState::DISCONNECTED) {
    // attempt connection
    fprintf(stderr, "WebQ: Connecting\n");

    CURLcode res = curl_easy_perform(curl);
    /* Check for errors */
    if(res != CURLE_OK) {
      fprintf(stderr, "WebQ: Failed to access %s: %s\n", endpoint,
              curl_easy_strerror(res));
      state = UpstreamState::DISCONNECTED_COOLDOWN;
      setDelay(DISCONNECTED_COOLDOWN_TIME);

      return;
    }

    fprintf(stderr, "WebQ: Connection successful\n");
    state = UpstreamState::IDLE;
  }

  if(state == UpstreamState::IDLE && outbound_message_waiting) {
    state = UpstreamState::SENDING;
    sending = to_send;
    sending_offset = 0;
    outbound_message_waiting = false;
  }

  if(state == UpstreamState::SENDING && delayExpired()) {
    uint8_t* msg = (uint8_t*)&sending;
    ssize_t wrote = write(fp, (const void*)&msg[sending_offset], sizeof(Message) - sending_offset);
    if(wrote == -1) {
      fprintf(stderr, "WebQ: write failed: (%d) %s\n", errno, strerror(errno));
      if(errno == EAGAIN || errno == EWOULDBLOCK) {
        setDelay(WRITE_AGAIN_COOLDOWN_TIME);
      } else if(errno == EBADF) {
        state = UpstreamState::DISCONNECTED;
      } else {
        // switch to a fail state so the user can decide what happens next
        state = UpstreamState::SENDING_FAILED;
        failure_acknowledged = false;
      }
      return;
    }
    sending_offset += wrote;
    if(sending_offset == sizeof(Message)) {
      state = UpstreamState::ACKING;
      last_sent = sending;
      ack_attempts = 0;
      receiving_offset = 0;
    }
  }

  if(state == UpstreamState::ACKING && delayExpired()) {
    uint8_t* msg = (uint8_t*)&receiving;
    ssize_t nread = read(fp, (void*)&msg[receiving_offset], sizeof(Message) - receiving_offset);
    if(nread == -1) {
      if(errno == EAGAIN || errno == EWOULDBLOCK) {
        setDelay(READ_AGAIN_COOLDOWN_TIME);
      } else if(errno == EBADF) {
        state = UpstreamState::DISCONNECTED;
      } else {
        fprintf(stderr, "WebQ: read failed: (%d) %s\n", errno, strerror(errno));
        state = UpstreamState::ACKING_FAILED;
        failure_acknowledged = false;
      }
      return;
    }
    receiving_offset += nread;
    if(receiving_offset == sizeof(Message)) {
      last_received = receiving;

      if(last_received.type == last_sent.type && last_received.id == last_sent.id) {
        state = UpstreamState::ACK_COMPLETE;
        ack_acknowledged = false;
      } else {
        ack_attempts++;
        receiving_offset = 0;

        /*
          fprintf(stderr, "Attempt %d: type %d vs %d, id %d vs %d\n", ack_attempts,
          last_received.type, last_sent.type,
          last_received.id, last_sent.id);
        */
        if(ack_attempts == MAX_ACK_ATTEMPTS) {
          state = UpstreamState::ACKING_FAILED;
          failure_acknowledged = false;
        } else {
          //fprintf(stderr, "read succeeded but wrong result, retry\n");
          setDelay(ACK_AGAIN_COOLDOWN_TIME);
        }
      }
    }
  }

  if(state == UpstreamState::ACK_COMPLETE && ack_acknowledged) {
    state = UpstreamState::IDLE;
  }

  if(state == UpstreamState::ACKING_FAILED && failure_acknowledged) {
    state = UpstreamState::IDLE;
  }

  if(state == UpstreamState::SENDING_FAILED && failure_acknowledged) {
    state = UpstreamState::IDLE;
  }
}

bool WebServiceFSM::send(const Message* message) {
  if(outbound_message_waiting) return false;

  to_send = *message;
  outbound_message_waiting = true;
  return true;
}

bool WebServiceFSM::acknowledgeAck() {
  if(ack_acknowledged) return false;

  ack_acknowledged = true;
  return true;
}

bool WebServiceFSM::clearError() {
  if(failure_acknowledged) return false;

  failure_acknowledged = true;
  return true;
}

bool WebServiceFSM::close() {
  if(fp == 0) return false;
  ::close(fp);
  state = UpstreamState::DISCONNECTED;
  return true;
}

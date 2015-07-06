#ifndef WSFSM_H
#define WSFSM_H

#include <stdint.h>
#include <curl/curl.h>
#include <ArduinoJson.h>

#include "protocol.h"
#include "systemtime.h"
#include "upstream.h"


/**
 * Web service class for the PI side of the Upstream finite state machine.
 * Portions of this are pure virtual so that the interface with the
 * webservice can be emulated or real.
 */

class WebServiceFSM : public UpstreamFSM {
 private:
  Message last_sent;
  Message last_received;

  Message sending;
  Message receiving;
  Message to_send;
  CURL *curl;
  CURLcode res;
  struct curl_slist *list;
  StaticJsonBuffer<200> jsonBuffer;

  const char* endpoint;
  int fp;
  uint8_t dev;
  uint8_t sending_offset;
  uint8_t receiving_offset;
  uint8_t ack_attempts;

  uint8_t outbound_message_waiting : 1;
  uint8_t ack_acknowledged : 1;
  uint8_t failure_acknowledged : 1;

 public:

  WebServiceFSM();
  ~WebServiceFSM();

  void putCmdStatus(long cid, bool status);
  void pullQueuedCmd();

  virtual void init(const char* endpoint);
  virtual void update();
  virtual bool send(const Message* message);
  virtual bool acknowledgeAck();
  virtual bool clearError();
  virtual bool close();
};

#endif

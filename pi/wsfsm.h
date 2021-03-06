#ifndef WSFSM_H
#define WSFSM_H

#include <stdint.h>

#include <curl/curl.h>

#include <ArduinoJson.h>

#include "protocol.h"
#include "systemtime.h"
#include "upstream.h"

#include "wsmsg.h"

/**
 * Web service class for the PI side of the Upstream finite state machine.
 * Portions of this are pure virtual so that the interface with the
 * webservice can be emulated or real.
 */

class WebServiceFSM : public UpstreamFSM {
 public:
  static constexpr auto MAX_MSG_SIZE = 512;


  // TODO - update per cpp
  WebServiceFSM();
  WebServiceFSM(const char* endpoint);
  ~WebServiceFSM();

  JsonObject& getJson(const char* endpoint);
  bool putJson(JsonObject& msg);
  bool postJson(JsonObject& msg);

  bool command_available;
  bool command_completed;
  WebServiceMsg command;

  void putCmdStatus(long cid, bool status);
  Message pullQueuedCmd();

  virtual void update();
  virtual bool send(const Message* message) { fprintf(stderr, "SEND UNIMPLEMENTED\n"); return false; };
  virtual bool acknowledgeAck() { fprintf(stderr, "ACK UNIMPLEMENTED\n"); return false; };
  virtual bool clearError() { fprintf(stderr, "CLEAR UNIMPLEMENTED\n"); return false; };
  virtual bool close() { fprintf(stderr, "CLOSE UNIMPLEMENTED\n"); return false; };

 private:
  CURL *curl;
  CURLcode res;
  struct curl_slist *list;
  StaticJsonBuffer<MAX_MSG_SIZE> jsonBuffer;

  bool transmitJson(const char* httpMethod, const char* endpoint, JsonObject& root);
};

#endif

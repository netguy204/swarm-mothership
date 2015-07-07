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
  CURL *curl;
  CURLcode res;
  struct curl_slist *list;
  WebServiceMsg command;

  bool command_available;
  bool command_completed;


 public:

  // TODO - update per cpp
  WebServiceFSM(const char* endpoint);
  ~WebServiceFSM();

  void putCmdStatus(long cid, bool status);
  Message pullQueuedCmd();

  virtual void update();
  virtual bool send(const Message* message);
  virtual bool acknowledgeAck();
  virtual bool clearError();
  virtual bool close();
};

#endif

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <asm/types.h>
#include <sys/ioctl.h>

#include "wsfsm.h"

WebServiceFSM::WebServiceFSM() {
  state = UpstreamState::DISCONNECTED;
  command_available = false;
  command_completed = false;

  curl_global_init(CURL_GLOBAL_ALL);
  curl = curl_easy_init();
  list = NULL;
   // The current server requires the data uploaded as app/json type
  list = curl_slist_append(list, "Content-type: application/json");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);
}

WebServiceFSM::~WebServiceFSM() {
  curl_slist_free_all(list);
  curl_easy_cleanup(curl);
  curl_global_cleanup();
}

bool WebServiceFSM::putJson(JsonObject& msg) {
  // .printTo stupidly writes a '\0' in its last position, so any destination
  // printed to must ask for one more than the strlen of the JSON.  At the
  // other end of the connection, Node.js will choke on PUT data if you throw a
  // null byte at it.  So, the FILE* must be as large as the JSON string, not
  // counting the null byte.
  // I place the primary blame on ArduinoJson for this one.  Honte, Benoit!
  char to_server[MAX_MSG_SIZE];
  msg.printTo(to_server, 1+msg.measureLength());
  auto fout = fmemopen(to_server, msg.measureLength(), "r");

  curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
  curl_easy_setopt(curl, CURLOPT_READDATA, fout);

  char from_server[MAX_MSG_SIZE];
  auto fin = fmemopen(from_server, MAX_MSG_SIZE, "w");
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fin);

  res = curl_easy_perform(curl);
  fclose(fin);
  fclose(fout);
  // Turn off PUT mode
  curl_easy_setopt(curl, CURLOPT_UPLOAD, 0);

  // Check for errors
  if(res != CURLE_OK) {
    fprintf(stderr, "WebQ: Failed to access %s: %s\n", "localhost",
        curl_easy_strerror(res));
  }
  else {
    fprintf(stderr, "WebQ: Connection successful, received '%s'\n", from_server);
  }

  return res == CURLE_OK;
}

bool WebServiceFSM::postJson(JsonObject& msg) {
  // .printTo stupidly writes a '\0' in its last position, so any destination
  // printed to must ask for one more than the strlen of the JSON.
  char to_server[MAX_MSG_SIZE];
  msg.printTo(to_server, 1+msg.measureLength());
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, to_server);

  char from_server[MAX_MSG_SIZE];
  auto fin = fmemopen(from_server, MAX_MSG_SIZE, "w");

  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fin);

  res = curl_easy_perform(curl);
  fclose(fin);

  // Check for errors
  if(res != CURLE_OK) {
    fprintf(stderr, "WebQ: Failed to access %s: %s\n", "localhost",
        curl_easy_strerror(res));
  }
  else {
    fprintf(stderr, "WebQ: Connection successful, received '%s'\n", from_server);
  }

  return res == CURLE_OK;
}

JsonObject& WebServiceFSM::getJson(const char* endpoint) {
  curl_easy_setopt(curl, CURLOPT_URL, endpoint);

  char from_server[MAX_MSG_SIZE];
  auto fin = fmemopen(from_server, MAX_MSG_SIZE, "w");

  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fin);

  res = curl_easy_perform(curl);
  fclose(fin);

  // Check for errors
  if(res != CURLE_OK) {
    fprintf(stderr, "WebQ: Failed to access %s: %s\n", "localhost",
        curl_easy_strerror(res));
    char failed[3] = "!!";
    return jsonBuffer.parseObject(failed); // so obj.success will be false
  }

  fprintf(stderr, "WebQ: Connection successful, received '%s'\n", from_server);

  return jsonBuffer.parseObject(from_server);
}

// TODO: need to be able to do both post (for sensor status) and put (for acking commands)
// hence  httpMethod
bool WebServiceFSM::transmitJson(const char* httpMethod, const char* endpoint, JsonObject& root) {

  curl_easy_setopt(curl, CURLOPT_URL, endpoint);

  if(!strcmp(httpMethod, "PUT")) {
    return putJson(root);
  }
  if(!strcmp(httpMethod, "POST")) {
    return postJson(root);
  }

  fprintf(stderr, "Unknown method '%s'\n", httpMethod);

  return false;
}

void WebServiceFSM::update() {

  if(state == UpstreamState::IDLE && !command_available && delayExpired()) {
    state = UpstreamState::FETCH_CMD;
  }
  if(state == UpstreamState::IDLE && command_completed) {
    state = UpstreamState::ACKING;
  }

  if(state == UpstreamState::ACKING && delayExpired()) {
    JsonObject& obj = jsonBuffer.createObject();
    command.toJson(obj);
    if (transmitJson("PUT","http://localhost:8080/commands",obj)) {
      state = UpstreamState::IDLE;
    } else {
      setDelay(READ_AGAIN_COOLDOWN_TIME); // which delay period to use?
    }
  }
  if(state == UpstreamState::FETCH_CMD) {
    auto& obj = getJson("http://localhost:8080/commands?pid=0");
    if (!obj.success()) {
      setDelay(READ_AGAIN_COOLDOWN_TIME);
      state = UpstreamState::IDLE;
    } else {
      if (command.fromJson(obj)) {
        command_available = true;
        command_completed = false;
        state = UpstreamState::IDLE;
      } else {
        setDelay(READ_AGAIN_COOLDOWN_TIME);
        state = UpstreamState::IDLE;
      }
    }
  }
  if(state == UpstreamState::SENDING && delayExpired()) {
    // to be implemented later, when mothership has sensor status
  }
}

void WebServiceFSM::putCmdStatus(long cid, bool status) {

  JsonObject& root = jsonBuffer.createObject();
  root["cid"] = 0; // mothership

  if (status) {
    root["complete"] = true;
  } else {
    root["complete"] = false;
  }


}


//cmd json syntax:
//if type=DRIVE speed, heading
Message WebServiceFSM::pullQueuedCmd() {

  char from_server[MAX_MSG_SIZE];
  auto fin = fmemopen(from_server, MAX_MSG_SIZE, "w");
  curl_easy_setopt(curl, CURLOPT_WRITEDATA, fin);

  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"pid\":0}");

  res = curl_easy_perform(curl);
  fclose(fin);

  if(res != CURLE_OK) {
    fprintf(stderr, "WebQ: Failed to access %s: %s\n", "localhost",
        curl_easy_strerror(res));
    //return 1;
  }
  fprintf(stderr, "WebQ: Connection successful\n");
  fprintf(stderr, "%s\n", from_server);
  auto& root = jsonBuffer.parseObject(from_server);

  if (!root.success()) {
    printf("json parsing failed.\n");
    // should return?
  }

  long pid = root["pid"];

  //  long pid = root["properties"]["id"];
  //  long cmdId = root["cid"]; // UNCOMMENT AFTER ADDED TO WEBSERVICE
  fprintf(stderr,"pid =  %ld\n",pid);
  Message msg;
  // TEMP
  int8_t speed_ival = 1;
  int8_t angle_ival = 1;
  int id = 0;
  // end TEMP
  messageSignedInit(&msg, COMMAND_SET_MOTION, speed_ival, angle_ival, id++);
  return msg;
}

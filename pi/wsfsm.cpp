#include <asm/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "wsfsm.h"

static char cmdFromQueue[256];

WebServiceFSM::WebServiceFSM() {

  state = UpstreamState::DISCONNECTED;
  outbound_message_waiting = false;
  ack_acknowledged = false;
  failure_acknowledged = false;

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

// callback that curl uses to capture the read data instead of 
// printing it to the screen
size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
  size_t n = 0;
  while(n < size*nmemb) {
    fprintf(stderr, "%c", (char)((char*)buffer)[n++]);
  }
  strncpy(cmdFromQueue,(char*)buffer,size*nmemb);
  cmdFromQueue[size*nmemb] = '\0';
  return nmemb;
}

// TODO: need to be able to do both post (for sensor status) and put (for acking commands)
// hence  httpMethod
bool transmitJson(const char* httpMethod, const char* endpoint, JsonObject& root) {

  curl_easy_setopt(curl, CURLOPT_URL, endpoint);
  // Add a byte at both the allocation and printing steps
  // for the NULL.
  char *buf = (char*)malloc(1 + root.measureLength());
  root.printTo(buf, 1+root.measureLength());
  printf("%s\n", buf);

  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, buf);
  // curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"id\":0,\"long\":1}");

  CURLcode res = curl_easy_perform(curl);
  /* Check for errors */
  if(res != CURLE_OK) {
    fprintf(stderr, "WebQ: Failed to access %s: %s\n", "localhost",
        curl_easy_strerror(res));
    free(buf);
    return false;
  }
  fprintf(stderr, "WebQ: Connection successful\n");
  free(buf);
  return true;
}

JsonObject& fetchJson(StaticJsonBuffer<200>& jsonBuffer, const char* endpoint)
{
  curl_easy_setopt(curl, CURLOPT_URL, endpoint);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
  CURLcode res = curl_easy_perform(curl);
 
  if(res != CURLE_OK) {
    fprintf(stderr, "WebQ: Failed to access %s: %s\n", "localhost",
        curl_easy_strerror(res));
    return jsonBuffer.parseObject("!!"); // so obj.success will be false
   }
  fprintf(stderr, "%s\n", cmdFromQueue);
  JsonObject& root = jsonBuffer.parseObject(cmdFromQueue);
  return root;

  }

void WebServiceFSM::update() {
 
  if(state == UpstreamState:IDLE && !command_available && delayExpired()) {
    state = UpstreamState::FETCH_CMD;
  }
  if(state == UpstreamState:IDLE && command_complete) {
    state = UpstreamState::ACKING;
  }

  if(state == UpstreamState::ACKING && delayExpired()) {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& obj = jsonBuffer.createObject();
    command.toJson(obj);
    if (transmitJson("PUT","http://localhost:8080/commands",obj)) {
      state = UpstreamState::IDLE;
    } else {
      setDelay(READ_AGAIN_COOLDOWN_TIME); // which delay period to use?
    }
  }
  if(state == UpstreamState:FETCH_CMD) {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& obj = fetchJson(jsonBuffer, "http://localhost:8080/commands?pid=0");
    if (!obj.success()) {
      setDelay(READ_AGAIN_COOLDOWN_TIME); 
      state = UpstreamState::IDLE;
    } else {
      if (command.fromJson(obj)) {
        command_available = true;
        command_complete = false;
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

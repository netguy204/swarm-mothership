#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <asm/types.h>
#include <sys/ioctl.h>

#include "wsfsm.h"

static char cmdFromQueue[256];

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

bool WebServiceFSM::putJson(JsonObject& msg) {
  char status[MAX_MSG_SIZE];

  // .printTo stupidly writes a '\0' in its last position, so any destination
  // printed to must ask for one more than the strlen of the JSON.  At the
  // other end of the connection, Node.js will choke on PUT data if you throw a
  // null byte at it.  So, the FILE* must be as large as the JSON string, not
  // counting the null byte.
  // I place the primary blame on ArduinoJson for this one.  Honte, Benoit!
  msg.printTo(status, 1+msg.measureLength());
  auto buf = fmemopen(status, msg.measureLength(), "r");

  struct curl_slist *list = NULL;
   // The current server requires the data uploaded as app/json type
  list = curl_slist_append(list, "Content-type: application/json");
  curl_easy_setopt(curl, CURLOPT_HTTPHEADER, list);

  curl_easy_setopt(curl, CURLOPT_UPLOAD, 1);
  curl_easy_setopt(curl, CURLOPT_READDATA, buf);

  res = curl_easy_perform(curl);
  curl_slist_free_all(list);
  fclose(buf);

  // Check for errors
  if(res != CURLE_OK) {
    fprintf(stderr, "WebQ: Failed to access %s: %s\n", "localhost",
        curl_easy_strerror(res));
  }
  else {
    fprintf(stderr, "WebQ: Connection successful\n");
  }

  return res == CURLE_OK;
}


// TODO: need to be able to do both post (for sensor status) and put (for acking commands)
// hence  httpMethod
bool WebServiceFSM::transmitJson(const char* httpMethod, const char* endpoint, JsonObject& root) {

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
  // Check for errors
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

JsonObject& WebServiceFSM::fetchJson(StaticJsonBuffer<200>& jsonBuffer, const char* endpoint)
{
  curl_easy_setopt(curl, CURLOPT_URL, endpoint);
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
  CURLcode res = curl_easy_perform(curl);

  if(res != CURLE_OK) {
    fprintf(stderr, "WebQ: Failed to access %s: %s\n", "localhost",
        curl_easy_strerror(res));
    char failed[3] = "!!";
    return jsonBuffer.parseObject(failed); // so obj.success will be false
   }
  fprintf(stderr, "%s\n", cmdFromQueue);
  JsonObject& root = jsonBuffer.parseObject(cmdFromQueue);
  return root;

  }

void WebServiceFSM::update() {

  if(state == UpstreamState::IDLE && !command_available && delayExpired()) {
    state = UpstreamState::FETCH_CMD;
  }
  if(state == UpstreamState::IDLE && command_completed) {
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
  if(state == UpstreamState::FETCH_CMD) {
    StaticJsonBuffer<200> jsonBuffer;
    JsonObject& obj = fetchJson(jsonBuffer, "http://localhost:8080/commands?pid=0");
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

  StaticJsonBuffer<200> jsonBuffer;
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

  StaticJsonBuffer<200> jsonBuffer;
  curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
  curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "{\"pid\":0}");
  CURLcode res = curl_easy_perform(curl);

  if(res != CURLE_OK) {
    fprintf(stderr, "WebQ: Failed to access %s: %s\n", "localhost",
        curl_easy_strerror(res));
    //return 1;
  }
  fprintf(stderr, "WebQ: Connection successful\n");
  fprintf(stderr, "%s\n", cmdFromQueue);
  JsonObject& root = jsonBuffer.parseObject(cmdFromQueue);

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

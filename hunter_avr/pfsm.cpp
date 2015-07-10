#include "pfsm.h"
#include "swarm_config.h"

// will be overwritten with our actual swarm id at runtime
char command_endpoint[] = "/commands?pid=000";

void insertBase10(char* string, uint8_t value) {
  char buf[3];
  uint8_t idx = 0;
  do {
    buf[idx] = '0' + value % 10;
    value /= 10;
    idx++;
  } while(value > 0);
  
  for(uint8_t ii = 0; ii < idx; ++ii) {
    string[ii] = buf[idx - ii - 1];
  }
  
  string[idx] = '\0';
}

const char* const ProtocolFSM::StateStr[STATE_MAX+1] = {
  STATES(MAKE_STRING)
};

ProtocolFSM::ProtocolFSM(Stream& serial, const char* ssid, const char* password, const char* server, uint16_t port)
    : ssid(ssid), password(password), server(server), port(port), delay_end(0),
    serial(serial), esp(&serial, 4), rest(&esp), state(POWER_ON),
    wifi_connected(false), status_pending(false), command_valid(false), command_complete(false) {
      
  // hold the device in reset
  pinMode(RESET_PIN, OUTPUT);
  digitalWrite(RESET_PIN, LOW);
}

void ProtocolFSM::wifiCallback(void* response) {
  uint32_t status;
  RESPONSE res(response);

  if(res.getArgc() == 1) {
    res.popArgs((uint8_t*)&status, 4);
    if(status == STATION_GOT_IP) {
      wifi_connected = true;
    } else {
      wifi_connected = false;
    }
  }
}

long ProtocolFSM::pendingDelay() {
  return delay_end - millis();
}

bool ProtocolFSM::delayComplete() {
  return millis() >= delay_end;
}

bool ProtocolFSM::readyCheckTime() {
  return millis() >= ready_check;
}

bool ProtocolFSM::commandCheck() {
  return millis() >= command_check;
}

bool ProtocolFSM::isResetTime() {
  return millis() >= reset_time;
}

void ProtocolFSM::resetResetTime() {
  reset_time = millis() + 30000;
}

void ProtocolFSM::resetReadyCheck() {
  ready_check = millis() + 5000;
}

void ProtocolFSM::update() {
  if(state > RESET) {
    esp.process();
  }
     
  if(state > CONNECTING_WIFI && !wifi_connected) {
    // we're newly disconnected disconnected
    state = DISCONNECTED_WIFI;
  }
  
  if(state > CONNECTING_WIFI && readyCheckTime()) {
    Serial.println(F("PFSM: Checking readyness"));
    resetReadyCheck();
    if(!esp.ready()) {
      state = ENABLE;
      wifi_connected = false;
    }
  }
  
  if(state > CONNECTING_WIFI && isResetTime()) {
    // nothing conclusive has happened to convince us we still
    // have comms so it's time to reset
    Serial.println(F("PFSM: Forcing reset"));
    resetResetTime();
    state = STARTUP;
  }
  
  if(state == POWER_ON) {
    // ESP12 is fairly high current so we want to give it time to stabalize
    // and give the other electronics a chance to stabalize as well
    digitalWrite(RESET_PIN, LOW);
    delay_end = millis() + 1000;
    state = POWER_ON_RESET;
        
    // correct our swarm id in the endpoint string
    insertBase10(&command_endpoint[sizeof(command_endpoint) - 4], swarmID());
    Serial.println(command_endpoint);
  }
  
  if(state == POWER_ON_RESET && delayComplete()) {
    digitalWrite(RESET_PIN, HIGH);
    state = STARTUP;
  }
  
  if(state == STARTUP) {
    resetResetTime();
    esp.enable();
    delay_end = millis() + 500;
    state = ENABLE;
  }
  
  if(state == ENABLE && delayComplete()) {
    resetResetTime();
    esp.reset();
    delay_end = millis() + 500;
    state = RESET;
  }
  
  if(state == RESET && delayComplete() && esp.ready()) {
    resetResetTime();
    resetReadyCheck();
    state = DISCONNECTED_WIFI;
  }

  
  if(state == DISCONNECTED_WIFI) {
    esp.wifiCb.attach(this, &ProtocolFSM::wifiCallback);
    esp.wifiConnect(ssid, password);
    state = CONNECTING_WIFI;
  }
  
  if(state == CONNECTING_WIFI && wifi_connected) {
    state = DISCONNECTED_REST;
  }
  
  if(state == DISCONNECTED_REST) {
    if(rest.begin(server, port, false)) {
      state = IDLE;
    }
  }
  
  if(state == IDLE && !command_valid && commandCheck()) {
    state = FETCH_COMMAND;
  }
  
  if(state == IDLE && command_complete) {
    state = ACK_COMMAND;
  }
  
  if(state == IDLE && status_pending) {
    state = SENDING_STATUS;
    status_pending = false;
    
    StaticJsonBuffer<256> jsonBuffer;
    char buffer[256];
    
    JsonObject& obj = jsonBuffer.createObject();
    status.toJson(obj);
    obj.printTo(buffer, sizeof(buffer));
    rest.setContentType("application/json");
    rest.put("/status", buffer);
    
    // prepare for response
    rest.getResponse(NULL, 0, true);
    delay_end = millis() + 1000;
    state = SENDING_STATUS;
  }

  if(state == SENDING_STATUS) {
    char buffer[128];
    int resp;
    if((resp = rest.getResponse(buffer, sizeof(buffer), false)) == HTTP_STATUS_OK) {
      Serial.println(F("PFSM: Status delivered"));
      resetResetTime();
      resetReadyCheck();
      state = IDLE;
    } else if(delayComplete()) {
      Serial.println(F("PFSM: Status timed out"));
      state = IDLE;
    }
  }

  if(state == FETCH_COMMAND) {
    rest.get(command_endpoint);
    
    // prepare for response
    rest.getResponse(NULL, 0, true);
    delay_end = millis() + 1000;
    state = AWAITING_COMMAND;
  }
  
  if(state == AWAITING_COMMAND) {
    StaticJsonBuffer<256> jsonBuffer;
    char buffer[256];
    int resp;
    if((resp = rest.getResponse(buffer, sizeof(buffer), false)) == HTTP_STATUS_OK) {    
      // parse the response
      resetResetTime();
      resetReadyCheck();
      char* ptr = buffer;
      // skip to the payload
      for(uint16_t ii = 0; ii < sizeof(buffer); ++ii) {
        if(buffer[ii] == '\n') {
          ptr = &buffer[ii+1];
          break;
        }
      }
      JsonArray& root = jsonBuffer.parseArray(ptr);
      if(!root.success()) {
        Serial.print(F("PFSM: Failed to parse "));
        Serial.println(ptr);
        command_check = millis() + 1000;
        state = IDLE;
      } else {
        if(root.size() == 0) {
          // nothing in our queue
          Serial.println(F("No commands waiting"));
          command_check = millis() + 1000;
          state = IDLE;
        } else {
          if(command.fromJson(root[0])) {
            command_valid = true;
            command_complete = false;
            state = IDLE;
          } else {
            Serial.print(F("PFSM: Message invalid "));
            Serial.println(buffer);
            command_check = millis() + 1000;
            state = IDLE;
          }
        }
      }
    } else if(delayComplete()) {
      Serial.println(F("PFSM: Command request timed out"));
      command_check = millis() + 1000;
      state = IDLE;
    } 
  }
  
  if(state == ACK_COMMAND) {
    StaticJsonBuffer<128> jsonBuffer;
    char buffer[128];
    
    JsonObject& obj = jsonBuffer.createObject();
    obj["pid"] = swarmID();
    obj["cid"] = command.cid;
    obj["complete"] = true;
    obj.printTo(buffer, sizeof(buffer));
    
    rest.put(command_endpoint, buffer);
    
    // prepare for response
    rest.getResponse(NULL, 0, true);
    delay_end = millis() + 1000;
    state = AWAITING_ACK;
  }
  
  if(state == AWAITING_ACK) {
    char buffer[128];
    int resp;
    if((resp = rest.getResponse(buffer, sizeof(buffer), false)) == HTTP_STATUS_OK) {    
      // assume we got goodness
      state = IDLE;
      command_valid = false;
      command_complete = false;
    } else if(delayComplete()) {
      // this is the one case we retry forever. It's important that the mothership know that
      // we did what we said we would do.
      Serial.println(F("PFSM: Ack timed out"));
      state = ACK_COMMAND;
    }
  }
}

void ProtocolFSM::sendStatus(const SensorStatus& _status) {
  if(!status_pending) {
    status = _status;
    status_pending = true;
  }
}

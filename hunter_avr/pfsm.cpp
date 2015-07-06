#include "pfsm.h"


const char* const ProtocolFSM::StateStr[STATE_MAX+1] = {
  STATES(MAKE_STRING)
};

ProtocolFSM::ProtocolFSM(Stream& serial, const char* ssid, const char* password, const char* server, uint16_t port)
    : ssid(ssid), password(password), server(server), port(port), delay_end(0),
    serial(serial), esp(&serial, 4), rest(&esp), state(POWER_ON),
    wifi_connected(false), status_pending(false) {
      
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
    Serial.println("PFSM: Checking readyness");
    resetReadyCheck();
    if(!esp.ready()) {
      state = ENABLE;
      wifi_connected = false;
    }
  }
  
  if(state > CONNECTING_WIFI && isResetTime()) {
    // nothing conclusive has happened to convince us we still
    // have comms so it's time to reset
    Serial.println("PFSM: Forcing reset");
    resetResetTime();
    state = STARTUP;
  }
  
  if(state == POWER_ON) {
    // ESP12 is fairly high current so we want to give it time to stabalize
    // and give the other electronics a chance to stabalize as well
    digitalWrite(RESET_PIN, LOW);
    delay_end = millis() + 1000;
    state = POWER_ON_RESET;
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
    
    if(rest.getResponse(buffer, sizeof(buffer), true) == HTTP_STATUS_OK) {
      Serial.println("PFSM: wow, that was fast");
      resetResetTime();
      resetReadyCheck();
      state = IDLE;
    } else {
      delay_end = millis() + 1000;
      state = SENDING_STATUS;
    }
  }
  
      // getResponse needs first dibs on calling process if it needs t
  if(state == SENDING_STATUS) {
    char buffer[128];
    int resp;
    if((resp = rest.getResponse(buffer, sizeof(buffer), false)) == HTTP_STATUS_OK) {
      Serial.println("PFSM: Status delivered");
      resetResetTime();
      resetReadyCheck();
      state = IDLE;
    } else if(delayComplete()) {
      Serial.println("PFSM: Status timed out");
      state = IDLE;
    }
  }    
}

void ProtocolFSM::sendStatus(const SensorStatus& _status) {
  if(!status_pending) {
    status = _status;
    status_pending = true;
  }
}

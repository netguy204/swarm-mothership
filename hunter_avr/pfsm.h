#ifndef PFSM_H
#define PFSM_H

#include "messages.h"
#include "common.h"
#include <espduino.h>
#include "custom_rest.h"
#include <ArduinoJson.h>

#define RESET_PIN 2

#define STATES(m)       \
  m(POWER_ON),          \
  m(POWER_ON_RESET),    \
  m(STARTUP),           \
  m(ENABLE),            \
  m(RESET),             \
  m(DISCONNECTED_WIFI), \
  m(CONNECTING_WIFI),   \
                        \
  m(DISCONNECTED_REST), \
  m(CONNECTING_REST),   \
                        \
  m(IDLE),              \
                        \
  m(FETCH_COMMAND),     \
  m(AWAITING_COMMAND),  \
                        \
  m(ACK_COMMAND),       \
  m(AWAITING_ACK),      \
                        \
  m(SENDING_STATUS),    \
  m(STATE_MAX)

class ProtocolFSM {
  public:
  
  enum ProtocolState {
    STATES(MAKE_ENUM)
  };
  
  static const char * const StateStr[STATE_MAX+1];
  
  private:
  const char* ssid;
  const char* password;
  const char* server;
  uint16_t port;
  
  unsigned long delay_end;
  unsigned long ready_check;
  unsigned long reset_time;
  unsigned long command_check;
  
  public:
  
  Stream& serial;
  ESP esp;
  CREST rest;
  ProtocolState state;
  SensorStatus status;
  DriveCommand command;
  
  uint8_t wifi_connected : 1;
  uint8_t status_pending : 1;
  uint8_t command_valid    : 1;
  uint8_t command_complete : 1;
  
  ProtocolFSM(Stream& serial, const char* ssid, const char* password, const char* server, uint16_t port);
  
  void wifiCallback(void* response);
  
  long pendingDelay();
  
  bool delayComplete();
  
  bool readyCheckTime();
  
  bool isResetTime();
  
  bool commandCheck();
    
  void resetResetTime();
  
  void resetReadyCheck();
  
  void update();
  
  void sendStatus(const SensorStatus& _status);
};

#endif



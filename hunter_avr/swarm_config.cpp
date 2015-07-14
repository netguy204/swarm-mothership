#include "swarm_config.h"
#include <Arduino.h>

static bool hw_resolved = false;
static uint8_t hwid;

uint8_t hardwareID() {
  if(!hw_resolved) {
    hwid = 0;
    for(uint8_t ii = 11; ii <= 13; ++ii) {
      hwid <<= 1;
      pinMode(ii, INPUT);
      digitalWrite(ii, HIGH); // enable pull up
      hwid |= digitalRead(ii);
      digitalWrite(ii, LOW); // disable pull up
    }
    hw_resolved = true;
  }
  return hwid;
}


uint8_t swarmID() {
  return hardwareID() + 100;
}


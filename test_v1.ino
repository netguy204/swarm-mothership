#include <SoftwareSerial.h>
#include <Wire.h>

#include "protocol.h"
#include "SMC.h"

#define rxPin 2    // pin 3 connects to SMC TX
#define txPin 3    // pin 4 connects to SMC RX
#define resetPin 6 // pin 5 connects to SMC nRST
#define errPin 7   // pin 6 connects to SMC ERR

// some motor limit IDs
#define FORWARD_ACCELERATION 5
#define REVERSE_ACCELERATION 9
#define DECELERATION 2

#define DIRPIN 8
#define SPEEDPIN 9
#define SLAVE_ADDRESS 0x04

struct ProtocolFSM {
  Message message;
  Message status;
  uint8_t message_bytes;
  uint8_t dropped_bytes;
  uint8_t state;
};

enum {
  P_READY,
  P_RECEIVING,
  P_MESSAGE_WAITING,
  P_SYNCHRONIZING
};

ProtocolFSM pfsm;

 
struct MothershipFSM {
  Message current;
  uint32_t start;
  uint8_t state;
};

enum {
  M_IDLE,
  M_EXECUTION,
  M_OVERRIDE,
  M_ERROR
};


MothershipFSM mfsm;

void protocolInit() {
  pfsm.message_bytes = 0;
  pfsm.dropped_bytes = 0;
  pfsm.state = P_READY;
}

void protocolReceive(int byteCount) {
  uint8_t* rbuf = (uint8_t*)(&pfsm.message);
  while(Wire.available()) {
    uint8_t next = Wire.read();
    if(pfsm.state == P_MESSAGE_WAITING) {
      // we can't handle a new message. drop it on the floor
      pfsm.dropped_bytes++;
    } else if(pfsm.state == P_READY) {
      // start receiving a new message
      pfsm.message_bytes = 0;
      pfsm.dropped_bytes = 0;
      pfsm.state = P_RECEIVING;
    }
    
    if(pfsm.state == P_RECEIVING) {
      rbuf[pfsm.message_bytes] = next;
      pfsm.message_bytes++;
      if(pfsm.message_bytes == sizeof(pfsm.message)) {
        pfsm.state = P_MESSAGE_WAITING;
      }
    } else if(pfsm.state == P_SYNCHRONIZING) {
      // drop and see if we're syncd
      pfsm.dropped_bytes++;
      if(pfsm.dropped_bytes % sizeof(pfsm.message) == 0) {
        pfsm.state = P_READY;
      }
    }
  }
}

void protocolSend() {
  Wire.write((uint8_t*)(&pfsm.status), sizeof(pfsm.status));
}

bool protocolHasMessage(Message* msg) {
  if(!pfsm.state == P_MESSAGE_WAITING) return false;
  
  *msg = pfsm.message;
  if(pfsm.dropped_bytes % sizeof(pfsm.message) == 0) {
    pfsm.state = P_READY;
  } else {
    pfsm.state = P_SYNCHRONIZING;
  }
}

void protocolSetStatus(Message* msg) {
  pfsm.status = *msg;
}

void protocolSetStatus(MessageType type, uint16_t payload) {
  messageInit(&pfsm.status, type, payload);
}

void mothershipInit() {
  mfsm.state = M_IDLE;
  mfsm.start = millis();
}

void smcInitialize() {
  SMC.reset();
  
  SMC.setMotorLimit(FORWARD_ACCELERATION, 4);
  SMC.setMotorLimit(REVERSE_ACCELERATION, 10);
  SMC.setMotorLimit(DECELERATION, 20);
  
  // clear the safe-start violation and let the motor run
  SMC.exitSafeStart();
}

void setup()
{
  // initialize our FSMs
  protocolInit();
  mothershipInit();
  
  Serial.begin(19200);    // for debugging (optional)
  SMC.begin(rxPin, txPin, errPin, resetPin);
  Wire.begin(SLAVE_ADDRESS);
  Wire.onReceive(protocolReceive);
  Wire.onRequest(protocolSend);
  
  // this lets us read the state of the SMC ERR pin (optional)
  pinMode(errPin, INPUT);
 
  smcInitialize();
  
  // enable pullup resistors on our input pins
  pinMode(DIRPIN, INPUT);
  digitalWrite(DIRPIN, HIGH);
  
  pinMode(SPEEDPIN, INPUT);
  digitalWrite(SPEEDPIN, HIGH);
}

void loop()
{
  Serial.print("state = ");
  Serial.println(mfsm.state);
  
  int speed = 500;
  bool userOverride = false;
  
  if(digitalRead(SPEEDPIN) == 0) {
    speed = 1500;
    userOverride = true;
  }
  if(digitalRead(DIRPIN) == 0) {
    speed = -speed;
    userOverride = true;
  }
  
  if (digitalRead(errPin) == HIGH && mfsm.state != M_ERROR) {
    // once all other errors have been fixed,
    // this lets the motors run again
    mfsm.state = M_ERROR;
    mfsm.start = millis();
    uint16_t error = SMC.getVariable(ERROR_STATUS);
    Serial.print("Error Status: 0x");
    Serial.println(error, HEX);
    if(error == 0) {
      // this error is recoverable
      smcInitialize();
    }
    protocolSetStatus(REPORT_MOTOR_ERROR, error);
  } else if(userOverride) {
    protocolSetStatus(REPORT_USER_OVERRIDE, speed);
    mfsm.state = M_OVERRIDE;
    SMC.setMotorSpeed(speed);
  } else {
    if(mfsm.state == M_OVERRIDE) {
      // user override complete
      protocolSetStatus(REPORT_NOOP, 0);
      mfsm.state = M_IDLE;
      SMC.setMotorSpeed(0);
    } else if(mfsm.state == M_IDLE) {
      if(protocolHasMessage(&mfsm.current)) {
        if(mfsm.current.type == COMMAND_SET_SPEED) {
          mfsm.state = M_EXECUTION;
          mfsm.start = millis();
          SMC.setMotorSpeed((int16_t)messagePayload(&mfsm.current));
        } else {
          Serial.print("ignoring message ");
          Serial.println(mfsm.current.type);
        }
      }
    } else if(mfsm.state == M_EXECUTION) {
      uint32_t now = millis();
      if(now < mfsm.start || (now - mfsm.start >= 5)) {
        SMC.setMotorSpeed(0);
        mfsm.state = M_IDLE;
      }
    } else if(mfsm.state == M_ERROR) {
      uint32_t now = millis();
      if(now < mfsm.start || (now - mfsm.start >= 100)) {
        SMC.setMotorSpeed(0);
        mfsm.state = M_IDLE;
        protocolSetStatus(REPORT_NOOP, 0);
      }
      SMC.exitSafeStart();
      SMC.reset();
    }
  }
 
 
  // write input voltage (in millivolts) to the serial monitor
  /*
  Serial.print("VIN = ");
  Serial.print(SMC.getVariable(INPUT_VOLTAGE));
  Serial.println(" mV");
  */
  // if an error is stopping the motor, write the error status variable
  // and try to re-enable the motor
  
  
  
}

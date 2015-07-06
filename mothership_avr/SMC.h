#ifndef SMC_H
#define SMC_H

#include <SoftwareSerial.h>

// some variable IDs
#define ERROR_STATUS 0
#define LIMIT_STATUS 3
#define TARGET_SPEED 20
#define INPUT_VOLTAGE 23
#define TEMPERATURE 24

#define rxPin 2    // pin 3 connects to SMC TX
#define txPin 3    // pin 4 connects to SMC RX
#define resetPin 6 // pin 5 connects to SMC nRST
#define errPin 7   // pin 6 connects to SMC ERR

class SMCClass {
private:
  SoftwareSerial smcSerial;

  int readByte();
  
  
public:
  SMCClass();
  
  void begin();
  void reset();
  void exitSafeStart();
  
  void setMotorSpeed(int16_t speed);
  unsigned char setMotorLimit(unsigned char  limitID, unsigned int limitValue);
  unsigned int getVariable(unsigned char variableID);
};

extern SMCClass SMC;

#endif

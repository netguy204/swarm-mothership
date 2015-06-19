#ifndef SMC_H
#define SMC_H

#include <SoftwareSerial.h>

// some variable IDs
#define ERROR_STATUS 0
#define LIMIT_STATUS 3
#define TARGET_SPEED 20
#define INPUT_VOLTAGE 23
#define TEMPERATURE 24


class SMCClass {
private:
  SoftwareSerial* smcSerial;
  char errPin, resetPin;
  
  int readByte();
  
  
public:
  SMCClass();
  ~SMCClass();
  
  void begin(char rxPin, char txPin, char errPin, char resetPin);
  void reset();
  void exitSafeStart();
  
  void setMotorSpeed(int16_t speed);
  unsigned char setMotorLimit(unsigned char  limitID, unsigned int limitValue);
  unsigned int getVariable(unsigned char variableID);
};

extern SMCClass SMC;

#endif

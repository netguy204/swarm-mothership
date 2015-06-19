#include "SMC.h"
#include <Arduino.h>

SMCClass SMC;

SMCClass::SMCClass() 
 : smcSerial(rxPin, txPin) {
}

void SMCClass::begin() {
  smcSerial.begin(19200);
  reset();
}

void SMCClass::reset() {
  pinMode(resetPin, OUTPUT);
  digitalWrite(resetPin, LOW);  // reset SMC
  delay(1);  // wait 1 ms
  pinMode(resetPin, INPUT);  // let SMC run again
  delay(5);
  smcSerial.write(0xAA);  // send baud-indicator byte
}

// read a serial byte (returns -1 if nothing received after the timeout expires)
int SMCClass::readByte()
{
  char c;
  if(smcSerial.readBytes(&c, 1) == 0){ return -1; }
  return (byte)c;
}
 
// required to allow motors to move
// must be called when controller restarts and after any error
void SMCClass::exitSafeStart()
{
  smcSerial.write(0x83);
}
 
// speed should be a number from -3200 to 3200
void SMCClass::setMotorSpeed(int speed)
{
  if (speed < 0)
  {
    smcSerial.write(0x86);  // motor reverse command
    speed = -speed;  // make speed positive
  }
  else
  {
    smcSerial.write(0x85);  // motor forward command
  }
  smcSerial.write(speed & 0x1F);
  smcSerial.write(speed >> 5);
}
 
unsigned char SMCClass::setMotorLimit(unsigned char  limitID, unsigned int limitValue)
{
  smcSerial.write(0xA2);
  smcSerial.write(limitID);
  smcSerial.write(limitValue & 0x7F);
  smcSerial.write(limitValue >> 7);
  return readByte();
}
 
// returns the specified variable as an unsigned integer.
// if the requested variable is signed, the value returned by this function
// should be typecast as an int.
unsigned int SMCClass::getVariable(unsigned char variableID)
{
  smcSerial.write(0xA1);
  smcSerial.write(variableID);
  return readByte() + 256 * readByte();
}

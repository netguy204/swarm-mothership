#include <SoftwareSerial.h>
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

#define PITX 10
#define PIRX 11

SoftwareSerial piSerial = SoftwareSerial(PIRX, PITX);
 
void setup()
{
  Serial.begin(19200);    // for debugging (optional)
  SMC.begin(rxPin, txPin, errPin, resetPin);
  Serial.println("hi");
  piSerial.begin(19200);
  Serial.println("hi2");
  // this lets us read the state of the SMC ERR pin (optional)
  pinMode(errPin, INPUT);
 
  SMC.setMotorLimit(FORWARD_ACCELERATION, 4);
  SMC.setMotorLimit(REVERSE_ACCELERATION, 10);
  SMC.setMotorLimit(DECELERATION, 20);
  
  // clear the safe-start violation and let the motor run
  SMC.exitSafeStart();
  
  // enable pullup resistors on our input pins
  pinMode(DIRPIN, INPUT);
  digitalWrite(DIRPIN, HIGH);
  
  pinMode(SPEEDPIN, INPUT);
  digitalWrite(SPEEDPIN, HIGH);
  
  sendMessage(piSerial, COMMAND_SYNC_STREAM, COMMAND_SYNC_STREAM);
}
 
void loop()
{
  int speed = 500;
  if(digitalRead(SPEEDPIN) == 0) {
    speed = 1500;
  }
  if(digitalRead(DIRPIN) == 0) {
    speed = -speed;
  }
  
  SMC.setMotorSpeed(speed); 
  
  /*
  if(piSerial.available()) {
    int v = piSerial.read();
    piSerial.write(v);
    Serial.write(v);
  }
  
  if(Serial.available()) {
    int v = Serial.read();
    piSerial.write(v);
    Serial.write(v);
  }
  */
  
  /*
  Message msg;
  if(hasMessage(&msg, piSerial)) {
    Serial.print("got message ");
    Serial.print(msg.type);
    Serial.print(": low byte = ");
    Serial.print(msg.payload_low);
    Serial.print(" high byte = ");
    Serial.println(msg.payload_high);
    sendMessage(piSerial, REPORT_CURRENT_SPEED, 0);
    
  }
  */
  
  // signed variables must be cast to ints:
  //Serial.println((int)getVariable(TARGET_SPEED));
  //delay(10000);
  //setMotorSpeed(-speed);  // full-speed reverse
  //Serial.println((int)getVariable(TARGET_SPEED));
  //delay(10000);
 
  // write input voltage (in millivolts) to the serial monitor
  /*
  Serial.print("VIN = ");
  Serial.print(SMC.getVariable(INPUT_VOLTAGE));
  Serial.println(" mV");
  */
  // if an error is stopping the motor, write the error status variable
  // and try to re-enable the motor
  
  if (digitalRead(errPin) == HIGH)
  {
    Serial.print("Error Status: 0x");
    Serial.println(SMC.getVariable(ERROR_STATUS), HEX);
    // once all other errors have been fixed,
    // this lets the motors run again
    SMC.exitSafeStart();
  }
  
}

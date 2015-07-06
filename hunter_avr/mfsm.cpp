#include <Wire.h>
#include "mfsm.h"
#include <Arduino.h>


ComplexCompensation::ComplexCompensation(const Vector<int>& forward, const Vector<int>& reverse, const Vector<int>& right, const Vector<int>& left, TracksFSM& tfsm)
  : forward(forward), reverse(reverse), right(right), left(left), stopped(Vector<int>(0, 0, 0)), tfsm(tfsm) {
}

const Vector<int>& ComplexCompensation::center() const {
  if(tfsm.leftCurrent() > 0 && tfsm.rightCurrent() > 0) {
    return forward;
  } else if(tfsm.leftCurrent() < 0 && tfsm.rightCurrent() < 0) {
    return reverse;
  } else if(tfsm.leftCurrent() > 0) {
    return right;
  } else if(tfsm.rightCurrent() > 0) {
    return left;
  } else {
    return stopped;
  }
}

SimpleCompensation::SimpleCompensation(const Vector<int>& _center)
: _center(_center) {
}

const Vector<int>& SimpleCompensation::center() const {
  return _center;
}

MagFSM::MagFSM(uint8_t address, MotorCompensation& mc) 
: updated_data(false), state(CONFIGURE), address(address), mc(mc) {
}

void MagFSM::ackData() {
  updated_data = false;
}

long MagFSM::pendingDelay() {
  return delay_end - millis();
}

bool MagFSM::delayComplete() {
  return millis() >= delay_end;
}

void MagFSM::update() {
  if(state == STARTUP) {
    // wait for voltages to rise
    delay_end = millis() + 1000;
  }
  
  if(state == CONFIGURE && delayComplete()) {
    //Put the HMC5883 IC into the correct operating mode
    Wire.beginTransmission(address); //open communication with HMC5883
    /*
    Wire.write(0x02); //select mode register
    Wire.write(0x00); //continuous measurement mode
    */
    
    Wire.write(0x00); // config register A
    Wire.write(0b01110000); // 8 averaged samples, 15 sps, no bias
    Wire.write(0b00000000); // maximum gain (resolution)
    Wire.write(0b10000000); // high speed, continuous measurement
    Wire.endTransmission();
    
    delay_end = millis() + 1000; // wait for sensor to settle
    accumulator = 0;
    hdg = 0;
    first_measurement = true;
    state = REQUESTING;
  }
  
  if(state == REQUESTING && delayComplete()) {
    //Tell the HMC5883L where to begin reading data
    Wire.beginTransmission(address); // I2C 7bit address of HMC5883
    Wire.write(0x03); //select register 3, X MSB register
    Wire.endTransmission();
    
    delay_end = millis() + 10; // timeout on getting a response
    state = RECEIVING;
  }
  
  if(state == RECEIVING) {
    // wait till Wire has buffered all we need
    Wire.requestFrom(address, 6);
    if(Wire.available() >= 6) {
      x = Wire.read()<<8; //X msb
      x |= Wire.read(); //X lsb
      z = Wire.read()<<8; //Z msb
      z |= Wire.read(); //Z lsb
      y = Wire.read()<<8; //Y msb
      y |= Wire.read(); //Y lsb
      
      Vector<int> v = Vector<int>(x, y, z) - mc.center();
      if(v.y == 0) {
        if(v.x < 0) {
          hdg = 180;
        } else {
          hdg = 0;
        }
      } else {
        hdg = atan2(v.x, v.y) * 180 / PI;
      }
      
      if(first_measurement) {
        accumulator = hdg;
        first_measurement = false;
      } else { 
        const double alpha = .5;
        accumulator = alpha * hdg + (1-alpha)*accumulator;
      }
      
      // keep interrupt noise low by limiting data rate to 10 Hz
      delay_end = millis() + 10;
      updated_data = true;
      state = REQUESTING;
    } else if(delayComplete()) {
      // we've waited too long, try to restart the sensor
      state = STARTUP;
    }
  }
}

double MagFSM::heading() {
  return hdg;
}

double MagFSM::filteredHeading() {
  return accumulator;
}

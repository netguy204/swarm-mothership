#ifndef COMMON_H
#define COMMON_H

#include <Stream.h>

#define MAKE_ENUM(v) v
#define MAKE_STRING(v) #v

#include <Arduino.h>


template<typename T>
class Vector {
  public:
  T x, y, z;
  
  Vector()
  : x(0), y(0), z(0) {
  }
  
  Vector(T x, T y, T z)
  : x(x), y(y), z(z) {
  }
  
  Vector vmin(const Vector& o) const {
    return Vector(min(x, o.x), min(y, o.y), min(z, o.z));
  }
  
  Vector vmax(const Vector& o) const {
    return Vector(max(x, o.x), max(y, o.y), max(z, o.z));
  }
  
  Vector operator-(const Vector& o) const {
    return Vector(x - o.x, y - o.y, z - o.z);
  }
  
  void printTo(Stream& s) const {
    s.print(x);
    s.print(", ");
    s.print(y);
    s.print(", ");
    s.print(z);
  }
};

template<int BINS>
class Profiler {
  public:
  uint16_t nstarts;  

#ifdef PROFILING
  unsigned long starts[BINS+1];
  unsigned long bins[BINS+1];
#endif
  
  Profiler() 
  : nstarts(0) {
  }
 
#ifdef PROFILING 
  void reset() {
    nstarts = 0;
  }
  
  void start() {
    if(nstarts == 0) {
      for(uint8_t ii = 0; ii <= BINS; ++ii) {
        bins[ii] = 0;
      }
    }
    nstarts++;
    starts[0] = millis();
  }
  
  void mark(uint8_t bin) {
    starts[bin] = millis();
    bins[bin] += (starts[bin] - starts[bin-1]);
  }
  
  void report() {
    for(uint8_t ii = 1; ii <= BINS; ++ii) {
      Serial.print(ii);
      Serial.print(": ");
      Serial.println(bins[ii]);
    }
  }
#else
  void reset() { }
  
  void start() { }
  
  void mark(uint8_t bin) { }
  
  void report() { }
#endif
};

#endif


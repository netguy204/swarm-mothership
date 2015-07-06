#ifndef GPSFSM_H
#define GPSFSM_H

#include "common.h"
#include <Stream.h>

#define MAX_LENGTH 512

#define  POSLLH_MSG  0x02
#define  SBAS_MSG    0x32
#define  VELNED_MSG  0x12
#define  STATUS_MSG  0x03
#define  SOL_MSG     0x06
#define  DOP_MSG     0x04
#define  DGPS_MSG    0x31

#define LONG(X)    *(long*)(&data[X])
#define ULONG(X)   *(unsigned long*)(&data[X])
#define INT(X)     *(int*)(&data[X])
#define UINT(X)    *(unsigned int*)(&data[X])

class GPSFSM {
  Stream& gps;
  Stream* debug;
  
  unsigned char  state, lstate, code, id, chk1, chk2, ck1, ck2;
   
  unsigned int  length, idx, cnt;
  
  unsigned char data[MAX_LENGTH];
  
  float enuXF[9];
  
  public:
  
  // resolved parameters
  char status;
  long lat, lon;
  Vector<int32_t> origin; // origin
  Vector<int32_t> ecef_pos;
  Vector<int32_t> ecef_vel;
  
  unsigned long lastTime;
  unsigned long lastTimeMillis;
  
  uint8_t updated_ll     : 1;
  uint8_t updated_time   : 1;
  uint8_t updated_status : 1;
  uint8_t updated_ecef   : 1;
  uint8_t valid_ll       : 1;
  uint8_t valid_ecef     : 1;
  uint8_t valid_enu_xf   : 1;
  
  GPSFSM(Stream& gps, Stream* debug = NULL);
  
  void begin();
  
  void ackLatLon();
  
  void ackTime();
  
  void ackStatus();
  
  void ackECEF();
  
  void enableMsg (unsigned char id, bool enable);
  
  void update();
  
  // gps disciplined millis  
  unsigned long gmillis() const;
  
     // Convert 1e-7 value packed into long into decimal format
  void printLatLon (long val);
  
  void printHex (unsigned char val);
  
  void sendCmd (unsigned char len, uint8_t data[]);
  
  bool toENU(Vector<int32_t>& enu, const Vector<int32_t>& point) const;
};


#endif



#include "gpsfsm.h"
#include "MatrixMath.h"

#include <stdint.h>
#include <Arduino.h>

GPSFSM::GPSFSM(Stream& gps, Stream* debug)
: gps(gps), debug(debug), lastTime(0), updated_ll(false), updated_time(false), updated_status(false), updated_ecef(false),
  valid_ll(false), valid_ecef(false), valid_enu_xf(false) {
    
}

void GPSFSM::begin() {
  // Modify these to control which messages are sent from module
  enableMsg(POSLLH_MSG, true);    // Enable position messages
  enableMsg(SBAS_MSG, true);      // Enable SBAS messages
  enableMsg(VELNED_MSG, true);    // Enable velocity messages
  enableMsg(STATUS_MSG, true);    // Enable status messages
  enableMsg(SOL_MSG, true);       // Enable soluton messages
  enableMsg(DOP_MSG, true);       // Enable DOP messages
  enableMsg(DGPS_MSG, true);     // Disable DGPS messages      
}

void GPSFSM::ackLatLon() {
  updated_ll = false;
}

void GPSFSM::ackTime() {
  updated_time = false;
}

void GPSFSM::ackStatus() {
  updated_status = false;
}

void GPSFSM::ackECEF() {
  updated_ecef = false;
}

void GPSFSM::enableMsg (unsigned char id, bool enable) {
  //               MSG   NAV   < length >  NAV
  uint8_t cmdBuf[] = {0x06, 0x01, 0x03, 0x00, 0x01, id, enable ? 1 : 0};
  sendCmd(sizeof(cmdBuf), cmdBuf);
}

unsigned long GPSFSM::gmillis() const {
  return (millis() - lastTimeMillis) + lastTime;
}

void GPSFSM::update() {
  if (gps.available()) {
    unsigned char cc = gps.read();
    switch (state) {
      case 0:    // wait for sync 1 (0xB5)
        ck1 = ck2 = 0;
        if (cc == 0xB5)
          state++;
        break;
      case 1:    // wait for sync 2 (0x62)
        if (cc == 0x62)
          state++;
        else
          state = 0;
        break;
      case 2:    // wait for class code
        code = cc;
        ck1 += cc;
        ck2 += ck1;
        state++;
        break;
      case 3:    // wait for Id
        id = cc;
        ck1 += cc;
        ck2 += ck1;
        state++;
        break;
      case 4:    // wait for length uint8_t 1
        length = cc;
        ck1 += cc;
        ck2 += ck1;
        state++;
        break;
      case 5:    // wait for length uint8_t 2
        length |= (unsigned int) cc << 8;
        ck1 += cc;
        ck2 += ck1;
        idx = 0;
        state++;
        if (length > MAX_LENGTH)
          state= 0;
        break;
      case 6:    // wait for <length> payload uint8_ts
        data[idx++] = cc;
        ck1 += cc;
        ck2 += ck1;
        if (idx >= length) {
          state++;
        }
        break;
      case 7:    // wait for checksum 1
        chk1 = cc;
        state++;
        break;
      case 8:    // wait for checksum 2
        chk2 = cc;
        bool checkOk = ck1 == chk1  &&  ck2 == chk2;
        if (checkOk) {
          switch (code) {
            case 0x01:      // NAV-
              // Add blank line between time groups
              if (lastTime != ULONG(0)) {
                lastTime = ULONG(0);
                lastTimeMillis = millis();
                
                updated_time = true;
                
                if(debug) {
                  debug->print(F("\nTime: "));
                  debug->println(ULONG(0), DEC);
                }
              }
              debug->print("NAV-");
              switch (id) {
                case 0x02:  // NAV-POSLLH
                  lon = LONG(4);
                  lat = LONG(8);
                  updated_ll = true;
                  if(status >= 2) valid_ll = true;
                  if(debug) {
                    debug->print(F("POSLLH: lon = "));
                    printLatLon(lon);
                    debug->print(F(", lat = "));
                    printLatLon(lat);
                    debug->print(F(", vAcc = "));
                    debug->print(ULONG(24), DEC);
                    debug->print(F(" mm, hAcc = "));
                    debug->print(ULONG(20), DEC);
                    debug->print(F(" mm"));
                  }
                 break;
                case 0x03:  // NAV-STATUS
                  status = data[4];
                  updated_status = true;
                  
                  if(debug) {
                    debug->print(F("STATUS: gpsFix = "));
                    debug->print(status, DEC);
                    if (data[5] & 2) {
                      debug->print(F(", dgpsFix"));
                    }
                  }
                  break;
                case 0x04:  // NAV-DOP
                  if(debug) {
                    debug->print(F("DOP:    gDOP = "));
                    debug->print((float) UINT(4) / 100, 2);
                    debug->print(F(", tDOP = "));
                    debug->print((float) UINT(8) / 100, 2);
                    debug->print(F(", vDOP = "));
                    debug->print((float) UINT(10) / 100, 2);
                    debug->print(F(", hDOP = "));
                    debug->print((float) UINT(12) / 100, 2);
                  }
                  break;
                case 0x06:  // NAV-SOL
                  if(debug) {
                    ecef_pos.x = LONG(12);
                    ecef_pos.y = LONG(16);
                    ecef_pos.z = LONG(20);
                    ecef_vel.x = LONG(28);
                    ecef_vel.y = LONG(32);
                    ecef_vel.z = LONG(36);
                    updated_ecef = true;
                    if(status >= 2) valid_ecef = true;
                    debug->print(F("SOL:    week = "));
                    debug->print(UINT(8), DEC);
                    debug->print(F(", gpsFix = "));
                    debug->print(data[10], DEC);
                    debug->print(F(", x = "));
                    debug->print(ecef_pos.x, DEC);
                    debug->print(F(", y = "));
                    debug->print(ecef_pos.y, DEC);
                    debug->print(F(", z = "));
                    debug->print(ecef_pos.z, DEC);
                    debug->print(F(", dx = "));
                    debug->print(ecef_vel.x, DEC);
                    debug->print(F(", dy = "));
                    debug->print(ecef_vel.y, DEC);
                    debug->print(F(", dz = "));
                    debug->print(ecef_vel.z, DEC);
                    debug->print(F(", vAcc = "));
                    debug->print(UINT(40), DEC);
                    debug->print(F(", pDOP = "));
                    debug->print((float) UINT(44) / 100.0, 2);
                    debug->print(F(", pAcc = "));
                    debug->print(ULONG(24), DEC);
                    debug->print(F(" cm, numSV = "));
                    debug->print(data[47], DEC);
                  }
                  break;
                case 0x12:  // NAV-VELNED
                  if(debug) {
                    debug->print(F("VELNED: gSpeed = "));
                    debug->print(ULONG(20), DEC);
                    debug->print(F(" cm/sec, sAcc = "));
                    debug->print(ULONG(28), DEC);
                    debug->print(F(" cm/sec, heading = "));
                    debug->print((float) LONG(24) / 100000, 2);
                    debug->print(F(" deg, cAcc = "));
                    debug->print((float) LONG(32) / 100000, 2);
                    debug->print(F(" deg"));
                  }
                  break;
                case 0x31:  // NAV-DGPS
                  if(debug) {
                    debug->print(F("DGPS:   age = "));
                    debug->print(LONG(4), DEC);
                    debug->print(F(", baseId = "));
                    debug->print(INT(8), DEC);
                    debug->print(F(", numCh = "));
                    debug->print(INT(12), DEC);
                  }
                  break;
                case 0x32:  // NAV-SBAS
                  if(debug) {
                  debug->print(F("SBAS:   geo = "));
                  switch (data[4]) {
                    case 133:
                      debug->print(F("Inmarsat 4F3"));
                      break;
                    case 135:
                      debug->print(F("Galaxy 15"));
                      break;
                    case 138:
                      debug->print(F("Anik F1R"));
                      break;
                    default:
                      debug->print(data[4], DEC);
                      break;
                    }
                    debug->print(F(", mode = "));
                    switch (data[5]) {
                      case 0:
                        debug->print(F("disabled"));
                        break;
                      case 1:
                        debug->print(F("enabled integrity"));
                        break;
                      case 2:
                        debug->print(F("enabled test mode"));
                        break;
                      default:
                        debug->print(data[5], DEC);
                    }
                    debug->print(F(", sys = "));
                     switch (data[6]) {
                      case 0:
                        debug->print(F("WAAS"));
                        break;
                      case 1:
                        debug->print(F("EGNOS"));
                        break;
                      case 2:
                        debug->print(F("MSAS"));
                        break;
                       case 16:
                        debug->print(F("GPS"));
                        break;
                     default:
                        debug->print(data[6], DEC);
                    }
                  }
                  break;
                default:
                  if(debug) printHex(id);
              }
              if(debug) debug->println();
              break;
            case 0x05:      // ACK-
              if(debug) debug->print(F("ACK-"));
              switch (id) {
                case 0x00:  // ACK-NAK
                if(debug)debug->print(F("NAK: "));
                break;
                case 0x01:  // ACK-ACK
                if(debug) debug->print(F("ACK: "));
                break;
              }
              if(debug) {
                printHex(data[0]);
                debug->print(" ");
                printHex(data[1]);
                debug->println();
              }
              break;
          }
        }
        state = 0;
        break;
    }
  }
  
  // can we produce a ENU transform?
  if(!valid_enu_xf && valid_ll && valid_ecef) {
    // declare our current position to be the origin
    origin = Vector<int32_t>(ecef_pos.x, ecef_pos.y, ecef_pos.z);
    
    // https://en.wikipedia.org/wiki/Geodetic_datum#From_ECEF_to_ENU
    float lambda = (lon * 1e-7) * PI / 180.0;
    float phi = (lat * 1e-7) * PI / 180.0;
    Serial.print("lambda = ");
    Serial.print(lambda);
    Serial.print(", phi = ");
    Serial.println(phi);
    
    float temp[] = {
      -sin(lambda),  -sin(phi) * cos(lambda),   cos(phi) * cos(lambda),
      cos(lambda),   -sin(phi) * sin(lambda),   cos(phi) * sin(lambda),
      0,             cos(phi),                  sin(phi)
    };
    memcpy(enuXF, temp, sizeof(temp));
    Matrix.Print(enuXF, 3, 3, "enuXF");
    
    valid_enu_xf = true; 
  }
}

bool GPSFSM::toENU(Vector<int32_t>& enu, const Vector<int32_t>& point) const {
  if(!valid_enu_xf) return false;
  
  Vector<int32_t> v = point - origin;
  
  float enuf[3];
  float vf[3] = {v.x, v.y, v.z};
  Matrix.Multiply(enuXF, vf, 3, 3, 1, enuf);
  enu = Vector<int32_t>(enuf[0], enuf[1], enuf[2]);
  
  return true;
}

   // Convert 1e-7 value packed into long into decimal format
void GPSFSM::printLatLon (long val) {
  debug->print(val/10000000.0f);
  /*
  char buffer[14];
  PString str(buffer, sizeof(buffer));
  str.print(val, DEC);
  char len = str.length();
  char ii = 0;
  while (ii < (len - 7)) {
    Serial.write(buffer[ii++]);
  }
  Serial.write('.');
  while (ii < len) {
    Serial.write(buffer[ii++]);
  }
  */
}

void GPSFSM::printHex (unsigned char val) {
  if (val < 0x10)
    debug->print("0");
  debug->print(val, HEX);
}

void GPSFSM::sendCmd (unsigned char len, uint8_t data[]) {
  gps.write(0xB5);
  gps.write(0x62);
  unsigned char chk1 = 0, chk2 = 0;
  for (unsigned char ii = 0; ii < len; ii++) {
    unsigned char cc = data[ii];
    gps.write(cc);
    chk1 += cc;
    chk2 += chk1;
  }
  gps.write(chk1);
  gps.write(chk2);
}

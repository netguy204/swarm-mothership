#ifndef WSMSG_H
#define WSMSG_H

#include <ArduinoJson.h>

class WebServiceMsg {

 public:
  long pid;
  uint16_t cid;
  double speed;
  double angle;
  bool completed;

  bool fromJson(JsonObject& obj);
  void toJson(JsonObject& obj);

};

#endif // WSMSG_H

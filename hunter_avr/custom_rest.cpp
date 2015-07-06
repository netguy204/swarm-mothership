/**
 * \file
 *       ESP8266 RESTful Bridge
 * \author
 *       Tuan PM <tuanpm@live.com>
 *
 * Customized for use by Huneters by Brian.Taylor@jhuapl.edu:
 * Primary modifications relate to the removal of wait states.
 */
#include "custom_rest.h"
CREST::CREST(ESP *e)
{
  esp = e;
  remote_instance = 0;
  timeout = DEFAULT_REST_TIMEOUT;
}
void CREST::restCallback(void *resp)
{
  response = true;
  res = resp;
}

boolean CREST::begin(const char* host, uint16_t port, boolean security)
{
  uint8_t sec = 0;
  if(security)
    sec = 1;
  restCb.attach(this, &CREST::restCallback);
  
  uint16_t crc = esp->request(CMD_REST_SETUP, (uint32_t)&restCb, 1, 3);
  crc = esp->request(crc,(uint8_t*)host, strlen(host));
  crc = esp->request(crc,(uint8_t*)&port, 2);
  crc = esp->request(crc,(uint8_t*)&sec, 1);
  esp->request(crc);

  if(esp->waitReturn(timeout) && esp->return_value != 0){
    remote_instance = esp->return_value;
    return true;
  }
  return false;
}

boolean CREST::begin(const char* host)
{
  return begin(host, 80, false);
}
void CREST::request(const char* path, const char* method, const char* data, int len)
{
  if(remote_instance == 0)
    return;
  uint16_t crc;
  if(len > 0)
    crc = esp->request(CMD_REST_REQUEST, 0, 0, 5);
  else
    crc = esp->request(CMD_REST_REQUEST, 0, 0, 3);
  crc = esp->request(crc,(uint8_t*)&remote_instance, 4);
  crc = esp->request(crc,(uint8_t*)method, strlen(method));
  crc = esp->request(crc,(uint8_t*)path, strlen(path));
  if(len > 0){
    crc = esp->request(crc,(uint8_t*)&len, 2);
    crc = esp->request(crc,(uint8_t*)data, len);
  }
    
  esp->request(crc);
}
void CREST::request(const char* path, const char* method, const char* data)
{
  request(path, method, data, strlen(data));
}
void CREST::get(const char* path, const char* data)
{
  request(path, "GET", data);
}
void CREST::get(const char* path)
{
  request(path, "GET", 0, 0);
}
void CREST::post(const char* path, const char* data)
{
  request(path, "POST", data);
}
void CREST::put(const char* path, const char* data)
{
  request(path, "PUT", data);
}
void CREST::del(const char* path, const char* data)
{
  request(path, "DELETE", data);
}

void CREST::setHeader(const char* value)
{
  uint8_t header_index = HEADER_GENERIC;
  uint16_t crc = esp->request(CMD_REST_SETHEADER, 0, 0, 3);
  crc = esp->request(crc,(uint8_t*)&remote_instance, 4);
  crc = esp->request(crc,(uint8_t*)&header_index, 1);
  crc = esp->request(crc,(uint8_t*)value, strlen(value));
  esp->request(crc);
}
void CREST::setContentType(const char* value)
{
  uint8_t header_index = HEADER_CONTENT_TYPE;
  uint16_t crc = esp->request(CMD_REST_SETHEADER, 0, 0, 3);
  crc = esp->request(crc,(uint8_t*)&remote_instance, 4);
  crc = esp->request(crc,(uint8_t*)&header_index, 1);
  crc = esp->request(crc,(uint8_t*)value, strlen(value));
  esp->request(crc);
}
void CREST::setUserAgent(const char* value)
{
  uint8_t header_index = HEADER_USER_AGENT;
  uint16_t crc = esp->request(CMD_REST_SETHEADER, 0, 0, 3);
  crc = esp->request(crc,(uint8_t*)&remote_instance, 4);
  crc = esp->request(crc,(uint8_t*)&header_index, 4);
  crc = esp->request(crc,(uint8_t*)value, strlen(value));
  esp->request(crc);
}
void CREST::setTimeout(uint32_t ms)
{
  timeout = ms;
}

uint16_t CREST::getResponse(char* data, uint16_t maxLen, bool firstPoll)
{
  if(firstPoll) {
    response = false;
  } else if(response){
    RESPONSE resp(res);

    resp.popArgs((uint8_t*)data, maxLen);
    return esp->return_value;
  }
    
  
  return 0;
}

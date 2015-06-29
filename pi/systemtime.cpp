#include "systemtime.h"
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

std::string stdstring(const char* message, ...) {
  va_list args;
  va_start(args, message);
  char buffer[1024];
  vsnprintf(buffer, sizeof(buffer), message, args);
  va_end(args);

  return std::string(buffer);
}

// http://www.guyrutenberg.com/2007/09/22/profiling-code-using-clock_gettime/
static timespec sum(const timespec& end, const timespec& start) {
  timespec temp;
  temp.tv_nsec = start.tv_nsec + end.tv_nsec;
  temp.tv_sec = start.tv_sec + end.tv_sec;
  if(temp.tv_nsec > 1000000000) {
    temp.tv_sec += 1;
    temp.tv_nsec -= 1000000000;
  }
  return temp;
}

static timespec diff(const timespec& end, const timespec& start) {
  timespec temp;
  if ((end.tv_nsec-start.tv_nsec)<0) {
    temp.tv_sec = end.tv_sec-start.tv_sec-1;
    temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
  } else {
    temp.tv_sec = end.tv_sec-start.tv_sec;
    temp.tv_nsec = end.tv_nsec-start.tv_nsec;
  }
  return temp;
}

inline static timespec timelength_to_timespec(double s) {
  timespec temp;
  temp.tv_sec = (time_t)s;
  temp.tv_nsec = (long)((s - temp.tv_sec) * 1000000000);
  return temp;
}

inline static double timespec_to_timelength(const timespec& spec) {
  return spec.tv_sec + 1e-9*spec.tv_nsec;
}


Time::Time(const timespec& spec)
  : spec(spec) {
}

Time::Time() {
  clock_gettime(CLOCK_MONOTONIC, &spec);
}

TimeLength Time::operator-(const Time& other) {
  return TimeLength::inSeconds(timespec_to_timelength(diff(spec, other.spec)));
}

Time Time::operator+(const TimeLength& other) {
  return Time(sum(spec, timelength_to_timespec(other.s)));
}

std::string Time::string() const {
  int ret;
  struct tm t;
  char buf[128];
  int len = sizeof(buf);

  tzset();
  if (localtime_r(&(spec.tv_sec), &t) == NULL)
    return std::string("error 1");

  ret = strftime(buf, len, "%F %T", &t);
  if (ret == 0)
    return std::string("error 2");
  len -= ret - 1;

  ret = snprintf(&buf[strlen(buf)], len, ".%09ld", spec.tv_nsec);
  if (ret >= len)
    return std::string("error 3");

  return std::string(buf);
}


TimeLength::TimeLength(double s)
    : s(s) {
}

TimeLength::TimeLength()
    : s(0) {
}

TimeLength TimeLength::inSeconds(double s) {
  return TimeLength(s);
}

TimeLength TimeLength::inMilliseconds(double s) {
  return TimeLength(s * 1e-3);
}

TimeLength TimeLength::inMicroseconds(double s) {
  return TimeLength(s * 1e-9);
}

std::string TimeLength::string() const {
  return stdstring("%f seconds", s);
}

double TimeLength::seconds() const {
  return s;
}

double TimeLength::milliseconds() const {
  return s * 1e3;
}

double TimeLength::microseconds() const {
  return s * 1e6;
}

TimeLength TimeLength::operator-(const TimeLength& other) {
  return TimeLength(s - other.s);
}

TimeLength TimeLength::operator+(const TimeLength& other) {
  return TimeLength(s + other.s);
}

TimeLength TimeLength::operator*(double scale) {
  return TimeLength(s * scale);
}

Time TimeLength::operator+(const Time& other) {
  return sum(timelength_to_timespec(s), other.spec);
}

bool TimeLength::operator==(const TimeLength& other) {
  return s == other.s;
}

bool TimeLength::operator>(const TimeLength& other) {
  return s > other.s;
}

bool TimeLength::operator<(const TimeLength& other) {
  return s < other.s;
}

bool TimeLength::operator>=(const TimeLength& other) {
  return s >= other.s;
}

bool TimeLength::operator<=(const TimeLength& other) {
  return s <= other.s;
}

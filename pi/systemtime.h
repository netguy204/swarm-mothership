#ifndef SYSTEMTIME_H
#define SYSTEMTIME_H

#include <time.h>
#include <string>

class Time;
class TimeLength;

class Time {
  friend class TimeLength;

 protected:
  timespec spec;
  Time(const timespec& spec);

 public:
  Time();

  TimeLength operator-(const Time& other);
  Time operator+(const TimeLength& other);

  std::string string() const;
};

class TimeLength {
  friend class Time;
 protected:
  double s;
  TimeLength(double s);

 public:
  TimeLength();

  static TimeLength inSeconds(double seconds);
  static TimeLength inMilliseconds(double ms);
  static TimeLength inMicroseconds(double us);

  double seconds() const;
  double milliseconds() const;
  double microseconds() const;
  std::string string() const;

  TimeLength operator-(const TimeLength& other);
  TimeLength operator+(const TimeLength& other);
  TimeLength operator*(double s);
  Time operator+(const Time& other);

  bool operator==(const TimeLength& other);
  bool operator>(const TimeLength& other);
  bool operator<(const TimeLength& other);
  bool operator>=(const TimeLength& other);
  bool operator<=(const TimeLength& other);
};


#endif

#ifndef TIME_MANAGER_CLASS_H
#define TIME_MANAGER_CLASS_H

#include <Arduino.h>

class TimeManagerClass {
public:
  void begin(long gmtOffsetSec = 0, int dstOffset = 0, const char* server = "pool.ntp.org");
  void waitForTime();
  String getTimestamp();
};

extern TimeManagerClass TimeManager;

#endif // TIME_MANAGER_CLASS_H

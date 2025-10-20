#ifndef SLEEP_MANAGER_H
#define SLEEP_MANAGER_H

#include <Arduino.h>

class SleepManager {
public:
  static void begin();
  static void deepSleep(uint64_t seconds);
};

#endif // SLEEP_MANAGER_H

#include "TimeManagerClass.h"
#include <time.h>

void TimeManagerClass::begin(long gmtOffsetSec, int dstOffset, const char* server) {
  configTime(gmtOffsetSec, dstOffset, server);
}

void TimeManagerClass::waitForTime() {
  struct tm timeinfo;
  int tries = 0;
  while (!getLocalTime(&timeinfo) && tries++ < 20) {
    Serial.println("⏳ Waiting for NTP time...");
    delay(500);
    yield();
  }
}

String TimeManagerClass::getTimestamp() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) return String("1970-01-01 00:00:00");
  char buf[25];
  strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
  return String(buf);
}

TimeManagerClass TimeManager;

#include "SleepManager.h"
#include "config.h"
#include <esp_sleep.h>

void SleepManager::begin() { /* place for wake-source setup if needed */ }

void SleepManager::deepSleep(uint64_t seconds) {
#if defined(USE_DEEP_SLEEP) && USE_DEEP_SLEEP
  Serial.printf("[SLEEP] Entering deep sleep for %llu seconds...\n", (unsigned long long)seconds);
  delay(100);
  esp_sleep_enable_timer_wakeup(seconds * 1000000ULL);
  esp_deep_sleep_start();
#else
  Serial.printf("[SLEEP] Deep sleep disabled; would sleep %llu seconds\n", (unsigned long long)seconds);
#endif
}

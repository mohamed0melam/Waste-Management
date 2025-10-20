#include <Arduino.h>
#include <unity.h>
#include "ConfigManager.h"

void test_config_apply_top_level() {
  ConfigManager cfg("A0221AU");
  cfg.begin();
  String json = "{\"timestamp\":\"2025-10-12 14:30:52\",\"BinHeight\":100,\"BinWidth\":50,\"BinLength\":50,\"PollingRate\":60}";
  bool ok = cfg.applyFromJson(json);
  TEST_ASSERT_TRUE(ok);
  TEST_ASSERT_EQUAL_UINT32(60000, cfg.getPollingMs());
  TEST_ASSERT_FLOAT_WITHIN(0.01, 100.0, cfg.getBinHeightCm());
}

void setup() {
  UNITY_BEGIN();
  RUN_TEST(test_config_apply_top_level);
  UNITY_END();
}

void loop() {}

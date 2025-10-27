#include <Arduino.h>
#include "Device.h"
#include "config.h"

static Device device; // single device instance

void setup() {
  Serial.begin(115200);
  delay(200);
  device.init();

  // If you prefer to perform runCycle immediately and then deep-sleep,
  // uncomment the following line and leave loop() mostly inactive.
  device.runCycle();
}

void loop() {
  // Call the device loop frequently to maintain MQTT and WiFi
  // device.loop();
  delay(200);
}

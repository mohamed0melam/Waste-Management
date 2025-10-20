#ifndef DEVICE_H
#define DEVICE_H

#include <Arduino.h>
#include "ConfigManager.h"
#include "SensorClass.h"
#include "MQTTManagerClass.h"
#include "WiFiManagerClass.h"
#include "TimeManagerClass.h"
#include "Telemetry.h"
#include "config.h"

class Device {
public:
  Device();
  void init();            // call from setup()
  void loop();            // call from loop()
  void runCycle();        // perform measurement, publish, sleep (call when ready)
// old: void onMqttMessage(char* topic, byte* payload, unsigned int length);
void onMqttMessage(char* topic, uint8_t* payload, unsigned int length);

private:
  ConfigManager _config;
  SensorClass _sensor;
  MQTTManagerClass _mqtt;
  WiFiManagerClass _wifi;
  unsigned long _rtc_counter;
  float _rtc_lastDistance;
  void publishTelemetry(float measuredDistance);
};

#endif // DEVICE_H

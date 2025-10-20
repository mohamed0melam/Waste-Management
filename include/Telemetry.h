#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <ArduinoJson.h>

class Telemetry {
public:
  static String createSimple(const String& id, float distance, unsigned long counter, const String& status);
};

#endif // TELEMETRY_H

#include "Telemetry.h"
#include "TimeManagerClass.h"
#include <esp_system.h>

String Telemetry::createSimple(const String& id, float distance, unsigned long counter, const String& status) {
  StaticJsonDocument<256> doc;
  doc["timestamp"] = TimeManager.getTimestamp();
  doc["device"] = id;
  doc["distance"] = distance;
  doc["counter"] = counter;
  doc["status"] = status;
  String out;
  serializeJson(doc, out);
  return out;
}

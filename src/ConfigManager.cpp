#include "ConfigManager.h"
#include <ArduinoJson.h>
#include <Preferences.h>

uint32_t ConfigManager::s_pollingRateSec = 0; // 0 = not set yet
const uint32_t ConfigManager::s_defaultPollingRateSec = 5;

ConfigManager::ConfigManager()
  : _pollingMs(s_defaultPollingRateSec * 1000UL),
    _binHeightCm(0.0f),
    _binWidthCm(0.0f),
    _binLengthCm(0.0f),
    _lastApplyChanged(false)
{}

void ConfigManager::begin() {
  Preferences prefs;
  if (prefs.begin("cfg", true)) { // read-only begin
    // Polling rate
    String sp = prefs.getString("poll", "0");
    uint32_t p = (uint32_t)atoi(sp.c_str());
    if (p > 0) {
      s_pollingRateSec = p;
      _pollingMs = p * 1000UL;
    } else {
      _pollingMs = s_defaultPollingRateSec * 1000UL;
    }

    // Bin dims
    String sbh = prefs.getString("bh", "0");
    String sbw = prefs.getString("bw", "0");
    String sbl = prefs.getString("bl", "0");
    _binHeightCm = sbh.toFloat();
    _binWidthCm  = sbw.toFloat();
    _binLengthCm = sbl.toFloat();

    prefs.end();
  } else {
    // cannot open prefs -> use defaults
    _pollingMs = s_defaultPollingRateSec * 1000UL;
  }
}

bool ConfigManager::applyFromJson(const String& json) {
  _lastApplyChanged = false;
  if (json.length() == 0) return false;

  StaticJsonDocument<256> doc;
  DeserializationError err = deserializeJson(doc, json);
  if (err) {
    Serial.printf("[ConfigManager] JSON parse failed: %s\n", err.c_str());
    return false;
  }

  // read current (old) values
  uint32_t oldPolling = s_pollingRateSec;
  float oldBh = _binHeightCm;
  float oldBw = _binWidthCm;
  float oldBl = _binLengthCm;

  // prepare new values starting from old (so missing keys do not count as change)
  uint32_t newPolling = oldPolling;
  float newBh = oldBh;
  float newBw = oldBw;
  float newBl = oldBl;

  // Extract values if present in JSON
  bool hasPolling = false, hasBh = false, hasBw = false, hasBl = false;

  if (doc.containsKey("PollingRate")) {
    hasPolling = true;
    if (doc["PollingRate"].is<unsigned long>() || doc["PollingRate"].is<int>()) {
      newPolling = doc["PollingRate"].as<unsigned long>();
    } else {
      const char* s = doc["PollingRate"];
      if (s) newPolling = (uint32_t)atoi(s);
    }
  }
  if (doc.containsKey("BinHeight")) {
    hasBh = true;
    newBh = (float)doc["BinHeight"].as<double>();
  }
  if (doc.containsKey("BinWidth")) {
    hasBw = true;
    newBw = (float)doc["BinWidth"].as<double>();
  }
  if (doc.containsKey("BinLength")) {
    hasBl = true;
    newBl = (float)doc["BinLength"].as<double>();
  }

  // Compare only the fields that were present in the JSON
  bool changed = false;
  if (hasPolling && (newPolling != oldPolling)) changed = true;
  if (hasBh && (fabs(newBh - oldBh) > FLOAT_EPS)) changed = true;
  if (hasBw && (fabs(newBw - oldBw) > FLOAT_EPS)) changed = true;
  if (hasBl && (fabs(newBl - oldBl) > FLOAT_EPS)) changed = true;

  // If nothing changed -> update in-memory for keys that were present (optional),
  // but avoid writing to NVS to save flash cycles.
  if (!changed) {
    Serial.println("[ConfigManager] applyFromJson: config unchanged - no persist.");
    // Still update in-memory for present keys (keeps runtime consistent)
    if (hasPolling) { s_pollingRateSec = newPolling; _pollingMs = newPolling * 1000UL; }
    if (hasBh) _binHeightCm = newBh;
    if (hasBw) _binWidthCm  = newBw;
    if (hasBl) _binLengthCm = newBl;
    _lastApplyChanged = false;
    return true;
  }

  // Some field changed -> persist all relevant values to NVS
  _lastApplyChanged = true;

  // Apply to memory
  if (hasPolling) { s_pollingRateSec = newPolling; _pollingMs = newPolling * 1000UL; }
  if (hasBh) _binHeightCm = newBh;
  if (hasBw) _binWidthCm  = newBw;
  if (hasBl) _binLengthCm = newBl;

  // Persist
  Preferences prefs;
  if (prefs.begin("cfg", false)) { // writable
    if (s_pollingRateSec > 0) prefs.putString("poll", String(s_pollingRateSec));
    prefs.putString("bh", String(_binHeightCm, 2));
    prefs.putString("bw", String(_binWidthCm, 2));
    prefs.putString("bl", String(_binLengthCm, 2));
    prefs.end();
    Serial.println("[ConfigManager] New config persisted to NVS.");
  } else {
    Serial.println("[ConfigManager] WARNING: failed to open Preferences for write!");
  }

  return true;
}

uint32_t ConfigManager::getPollingMs() const {
  if (s_pollingRateSec > 0) return s_pollingRateSec * 1000UL;
  return s_defaultPollingRateSec * 1000UL;
}

float ConfigManager::getBinHeightCm() const { return _binHeightCm; }
float ConfigManager::getBinWidthCm() const  { return _binWidthCm; }
float ConfigManager::getBinLengthCm() const { return _binLengthCm; }

void ConfigManager::setPollingRateSec(uint32_t s) {
  if (s == 0) return;
  s_pollingRateSec = s;
  Preferences prefs;
  if (prefs.begin("cfg", false)) {
    prefs.putString("poll", String(s_pollingRateSec));
    prefs.end();
  }
}

uint32_t ConfigManager::getPollingRateSec() {
  if (s_pollingRateSec > 0) return s_pollingRateSec;
  return s_defaultPollingRateSec;
}

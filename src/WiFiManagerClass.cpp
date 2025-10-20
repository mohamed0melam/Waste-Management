#include "WiFiManagerClass.h"
#include <WiFi.h>

WiFiManagerClass::WiFiManagerClass() {}
WiFiManagerClass::WiFiManagerClass(const char* ssid, const char* password) : _ssid(ssid), _password(password) {}

void WiFiManagerClass::begin() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(_ssid.c_str(), _password.c_str());
  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    Serial.print(".");
    delay(500);
    yield();
  }
  if (WiFi.status() == WL_CONNECTED) {
    _connectedAtMs = millis();
    Serial.printf("✅ WiFi connected, IP: %s\n", WiFi.localIP().toString().c_str());
  } else {
    Serial.println("⚠️ WiFi connect failed");
  }
}

void WiFiManagerClass::ensureConnected() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.disconnect(true, true);
    WiFi.begin(_ssid.c_str(), _password.c_str());
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
      Serial.print(".");
      delay(500);
      yield();
    }
    if (WiFi.status() == WL_CONNECTED) _connectedAtMs = millis();
  }
}

bool WiFiManagerClass::isConnected() { return WiFi.status() == WL_CONNECTED; }
String WiFiManagerClass::getSSID() const { return _ssid; }
String WiFiManagerClass::getIPAddress() const { return WiFi.localIP().toString(); }
String WiFiManagerClass::getMACAddress() const { return WiFi.macAddress(); }
uint32_t WiFiManagerClass::getUptimeSec() const { if (_connectedAtMs == 0) return 0; return (millis() - _connectedAtMs) / 1000; }
size_t WiFiManagerClass::getPasswordLength() const { return _password.length(); }

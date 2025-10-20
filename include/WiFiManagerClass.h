#ifndef WIFI_MANAGER_CLASS_H
#define WIFI_MANAGER_CLASS_H

#include <Arduino.h>

class WiFiManagerClass {
public:
  WiFiManagerClass();
  WiFiManagerClass(const char* ssid, const char* password);
  void begin();
  void ensureConnected();
  bool isConnected();
  String getSSID() const;
  String getIPAddress() const;
  String getMACAddress() const;
  uint32_t getUptimeSec() const;
  size_t getPasswordLength() const;
private:
  String _ssid;
  String _password;
  unsigned long _connectedAtMs = 0;
};

#endif // WIFI_MANAGER_CLASS_H

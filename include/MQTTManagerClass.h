#ifndef MQTT_MANAGER_CLASS_H
#define MQTT_MANAGER_CLASS_H

#include <Arduino.h>
#include <WiFi.h>
#include <PubSubClient.h>
#include <functional>

class MQTTManagerClass {
public:
  MQTTManagerClass();
  ~MQTTManagerClass();

  // Provide the actual network client (WiFiClient) here
  void begin(WiFiClient& networkClient, const char* server, int port, const char* user, const char* pass);

  // Accept std::function so lambdas with captures (e.g., [this]{...}) work
  void setCallback(std::function<void(char*, uint8_t*, unsigned int)> cb);

  void connect();
  bool connected();
  void loop();
  bool publish(const char* topic, const uint8_t* payload, size_t len, bool retained = false);
  bool subscribe(const char* topic);
  int state();
private:
  // NOTE: use pointer because PubSubClient has no default ctor that accepts Client later
  PubSubClient* _client;
  WiFiClient* _netClient;   // pointer to the WiFiClient provided in begin()
  const char* _server;
  int _port;
  const char* _user;
  const char* _pass;

  // store user callback
  std::function<void(char*, uint8_t*, unsigned int)> _userCb;

  // Bridge: a static function pointer matching PubSubClient callback signature
  static void internalCallback(char* topic, uint8_t* payload, unsigned int length);

  // single instance pointer used by the bridge (works for single MQTT instance)
  static MQTTManagerClass* s_instance;
};

#endif // MQTT_MANAGER_CLASS_H

#include "MQTTManagerClass.h"
#include "config.h"

MQTTManagerClass* MQTTManagerClass::s_instance = nullptr;

MQTTManagerClass::MQTTManagerClass()
  : _client(nullptr), _netClient(nullptr), _server(nullptr), _port(0), _user(nullptr), _pass(nullptr), _userCb(nullptr)
{}

MQTTManagerClass::~MQTTManagerClass() {
  if (_client) {
    delete _client;
    _client = nullptr;
  }
}

void MQTTManagerClass::begin(WiFiClient& networkClient, const char* server, int port, const char* user, const char* pass) {
  _netClient = &networkClient;
  _server = server; _port = port; _user = user; _pass = pass;

  // create PubSubClient using the provided WiFiClient
  if (_client) delete _client;
  _client = new PubSubClient(*_netClient);

  _client->setServer(_server, _port);

  // register the bridge callback function
  _client->setCallback(MQTTManagerClass::internalCallback);
  s_instance = this;
}

void MQTTManagerClass::setCallback(std::function<void(char*, uint8_t*, unsigned int)> cb) {
  _userCb = cb;
  // make sure the bridge is installed
  if (_client) _client->setCallback(MQTTManagerClass::internalCallback);
  s_instance = this;
}

void MQTTManagerClass::internalCallback(char* topic, uint8_t* payload, unsigned int length) {
  if (s_instance && s_instance->_userCb) {
    s_instance->_userCb(topic, payload, length);
  } else {
    Serial.printf("[MQTT] internalCallback: no user callback set (topic=%s)\n", topic);
  }
}

void MQTTManagerClass::connect() {
  if (!_client) {
    Serial.println("[MQTT] connect called but PubSubClient not initialized!");
    return;
  }
  if (_client->connected()) return;

  String clientId = String("ESP_") + String(DEVICE_ID);
  unsigned long start = millis();
  const unsigned long timeout = 15000UL;
  while (!_client->connected() && (millis() - start) < timeout) {
    Serial.print("[MQTT] Attempting connect...");
    if (_client->connect(clientId.c_str(), _user, _pass)) {
      Serial.println(" connected ✅");
      // allow delivery of retained messages
      _client->loop();
      delay(100);
      break;
    } else {
      int st = _client->state();
      Serial.printf(" failed (state=%d) => %d, retrying...\n", st, st);
      delay(500);
      yield();
    }
  }

  if (!_client->connected()) {
    Serial.printf("[MQTT] could not connect (state=%d)\n", _client->state());
  }
}

bool MQTTManagerClass::connected() {
  return _client && _client->connected();
}

void MQTTManagerClass::loop() {
  if (_client) _client->loop();
}

bool MQTTManagerClass::publish(const char* topic, const uint8_t* payload, size_t len, bool retained) {
  if (!_client) {
    Serial.println("[MQTT] publish called but client null");
    return false;
  }
  bool ok = _client->publish(topic, payload, len, retained);
  if (!ok) Serial.printf("[MQTT] publish failed topic=%s len=%u\n", topic, (unsigned)len);
  return ok;
}

bool MQTTManagerClass::subscribe(const char* topic) {
  if (!_client) {
    Serial.println("[MQTT] subscribe called but client null");
    return false;
  }
  bool ok = _client->subscribe(topic);
  Serial.printf("[MQTT] subscribe('%s') => %d\n", topic, ok ? 1 : 0);
  if (!ok) Serial.printf("[MQTT] subscribe failed state=%d\n", _client->state());
  return ok;
}

int MQTTManagerClass::state() {
  if (!_client) return -999;
  return _client->state();
}

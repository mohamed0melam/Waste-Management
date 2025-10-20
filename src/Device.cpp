#include "Device.h"
#include <WiFi.h>
#include <ArduinoJson.h>
#include <esp_system.h>
#include "SleepManager.h"
#include <esp_sleep.h>
#include <Preferences.h>

// retain across deep sleep resets:
RTC_DATA_ATTR static unsigned long rtc_counter = 0;
RTC_DATA_ATTR static float rtc_lastDistance = -1.0f;


Device::Device()
  : _sensor(SENSOR_RX_PIN), _wifi(WIFI_SSID, WIFI_PASSWORD), _rtc_counter(0), _rtc_lastDistance(-1.0f) {}

  static WiFiClient espTcpClient; // file-scope static so it survives the function


void Device::init() {
  Serial.println("=== Device::init ===");
  _config.begin();
  _sensor.begin();
  _wifi.begin();

  // Initialize MQTT with a real WiFiClient instance
  _mqtt.begin(espTcpClient, MQTT_SERVER, MQTT_PORT, MQTT_USER, MQTT_PASS);

  // set callback bound to this instance (matching uint8_t*)
  _mqtt.setCallback([this](char* topic, uint8_t* payload, unsigned int length) {
    this->onMqttMessage(topic, payload, length);
  });

  // Connect and subscribe
  _mqtt.connect();

  String cfgTopic = String(DEVICE_ID) + "/CONFIG";
  bool subOk = _mqtt.subscribe(cfgTopic.c_str());
  Serial.printf("[Device] subscribe result for %s => %d\n", cfgTopic.c_str(), subOk ? 1 : 0);

  // Wait briefly for retained CONFIG to arrive and be processed
  unsigned long t0 = millis();
  const unsigned long waitMs = 1000UL; // 1s
  while (millis() - t0 < waitMs) {
    _mqtt.loop();
    delay(20);
  }

  // setup time
  TimeManager.begin(3 * 3600, 0, "pool.ntp.org");
}


void Device::loop() {
  if (!_mqtt.connected()) _mqtt.connect();
  _mqtt.loop();
}

void Device::runCycle() {
  Serial.println("=== Device::runCycle ===");

  if (!_wifi.isConnected()) {
    _wifi.ensureConnected();
  }

  // ensure MQTT connected
  if (!_mqtt.connected()) _mqtt.connect();

  // measure
  float measuredDistance = -1.0f;
  const int MAX_ATTEMPTS = 4;
  for (int i = 0; i < MAX_ATTEMPTS; ++i) {
    measuredDistance = _sensor.readValidatedMedian();
    if (measuredDistance >= 0.0f) break;
    Serial.println("[Sensor] invalid reading - retrying...");
    delay(200);
    yield();
  }

  if (measuredDistance < 0.0f) {
    if (rtc_lastDistance > 0.0f) measuredDistance = rtc_lastDistance;
    else measuredDistance = -1.0f;
    Serial.printf("[Sensor] using fallback distance = %.2f\n", measuredDistance);
  } else {
    Serial.printf("[Sensor] distance = %.2f cm\n", measuredDistance);
  }

  rtc_counter++;
  rtc_lastDistance = measuredDistance;

  publishTelemetry(measuredDistance);

  uint32_t sleepSec = ConfigManager::getPollingRateSec();
  if (sleepSec == 0) sleepSec = 5;

  Serial.printf("[runCycle] Going to deep sleep for %u seconds...\n", (unsigned)sleepSec);
  delay(80);
  SleepManager::deepSleep((uint64_t)sleepSec);
}

void Device::publishTelemetry(float measuredDistance) {
  if (!_mqtt.connected()) {
    Serial.println("[publishTelemetry] MQTT not connected -> skipping publish");
    return;
  }

  float bh = _config.getBinHeightCm();
  if (bh <= 0.0f) bh = BIN_HEIGHT_CM;

  StaticJsonDocument<768> doc;
  doc["_id"] = String("DEVICE_") + String((uint64_t)millis() + esp_random());
  doc["Location"] = String(DEVICE_ID);
  doc["timestamp"] = TimeManager.getTimestamp();
  doc["distance_cm"] = measuredDistance;

  // --- Corrected Telemetry Calculation Block ---
  double height_cm = (double)bh;
  double width_cm  = (double)_config.getBinWidthCm();
  double length_cm = (double)_config.getBinLengthCm();
  double distance_cm = (double)measuredDistance;

  // default height if invalid
  if (height_cm <= 0.0) height_cm = 100.0;

  // sensor offset correction
  const double sensorOffsetCm = 0.0;
  double effectiveDistance = distance_cm - sensorOffsetCm;
  if (effectiveDistance < 0.0) effectiveDistance = 0.0;

  // filled height (cm)
  double filledHeightCm = height_cm - effectiveDistance;
  if (filledHeightCm < 0.0) filledHeightCm = 0.0;
  if (filledHeightCm > height_cm) filledHeightCm = height_cm;

  // convert cm → m
  double height_m = height_cm / 100.0;
  double width_m  = width_cm / 100.0;
  double length_m = length_cm / 100.0;
  double filledHeight_m = filledHeightCm / 100.0;

  // compute volumes in m³
  double totalVolume_m3 = 0.0;
  double filledVolume_m3 = 0.0;

  if (width_m > 0.0 && length_m > 0.0 && height_m > 0.0) {
    totalVolume_m3 = height_m * width_m * length_m;
    filledVolume_m3 = filledHeight_m * width_m * length_m;
  }

  // convert to liters (1 m³ = 1000 L)
  double totalVolumeLiters = totalVolume_m3 * 1000.0;
  double fillVolumeLiters  = filledVolume_m3 * 1000.0;

  // fill percentage
  double BinLevelPercent = 0.0;
  if (totalVolume_m3 > 0.0)
    BinLevelPercent = (filledVolume_m3 / totalVolume_m3) * 100.0;

  // clamp percentage
  if (BinLevelPercent < 0.0) BinLevelPercent = 0.0;
  if (BinLevelPercent > 100.0) BinLevelPercent = 100.0;

  // rounding helper
  auto round2 = [](double v)->double {
    return ((long)(v * 100.0 + (v >= 0 ? 0.5 : -0.5))) / 100.0;
  };

  // populate JSON
  doc["BinLevel"] = round2(BinLevelPercent);
  
  // --- Prevent overflow by sending large numbers as strings ---
  char volBuf[32];
  snprintf(volBuf, sizeof(volBuf), "%.2f", totalVolumeLiters);
  doc["BinVolumeLiters"] = volBuf;

  snprintf(volBuf, sizeof(volBuf), "%.2f", fillVolumeLiters);
  doc["BinFillVolumeLiters"] = volBuf;
  // ------------------------------------------------------------

  doc["BinHeightCm"] = bh;
  doc["BinWidthCm"] = _config.getBinWidthCm();
  doc["BinLengthCm"] = _config.getBinLengthCm();
  // --- End Corrected Block ---

  doc["counter"] = rtc_counter;
  doc["PowerMode"] = "Deep Sleep Measurement";
  doc["SSID"] = WiFi.SSID();
  doc["WiFiPassword"] = String("***");
  doc["NetworkIPaddress"] = WiFi.localIP().toString();
  doc["MACaddress"] = WiFi.macAddress();
  doc["NetworkUpTime"] = WiFi.isConnected() ? (millis() / 1000) : 0;
  doc["ControllerUpTime"] = (millis() / 1000);
  doc["MQTTBrokerIP"] = String(MQTT_SERVER);
  doc["MQTTBrokerClentID"] = String("ESP_") + String(DEVICE_ID);
  doc["PollingRate"] = ConfigManager::getPollingRateSec();

  static char payload[1024];
  size_t n = serializeJson(doc, payload, sizeof(payload));
  payload[n] = '\0';

  String topic = String(DEVICE_ID) + "/TELEMETRY";
  Serial.printf("[publishTelemetry] topic=%s payload_bytes=%u\n", topic.c_str(), (unsigned)n);
  
  bool ok = _mqtt.publish(topic.c_str(), (const uint8_t*)payload, n, false);
  if (ok) {
    Serial.printf("📤 Published to %s (len=%u)\n", topic.c_str(), (unsigned)n);
  } else {
    Serial.printf("⚠️ Publish FAILED to %s\n", topic.c_str());
  }

  unsigned long t0 = millis();
  while (millis() - t0 < 800) {
    _mqtt.loop();
    delay(20);
    yield();
  }
}

void Device::onMqttMessage(char* topic, uint8_t* payload, unsigned int length) {
  Serial.printf("[MQTT cb] topic=%s len=%u\n", topic, length);
  String json;
  json.reserve(length + 1);
  for (unsigned int i = 0; i < length; ++i) json += (char)payload[i];
  bool ok = _config.applyFromJson(json);
  if (ok) {
    Serial.println("[MQTT cb] Config applied from MQTT payload.");
    Serial.printf("  PollingRate=%u sec\n", ConfigManager::getPollingRateSec());
  } else {
    Serial.println("[MQTT cb] Failed to parse/apply config JSON.");
  }
}


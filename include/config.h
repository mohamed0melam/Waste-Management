#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// Default hardware / network configuration
#define WIFI_SSID     "I4TECH"
#define WIFI_PASSWORD "I4TECH@2023"

#define MQTT_SERVER   "185.194.217.124"
#define MQTT_PORT     1883
#define MQTT_USER     "pop"
#define MQTT_PASS     "4!9qp6qfr4^4b8!!*$$v"

#define DEVICE_ID     "A0221AU"
#define SENSOR_RX_PIN 20

// Fallback compile-time bin height (cm)
#define BIN_HEIGHT_CM 250.0f

// If you want to disable deep-sleep for debugging, undefine this
#define USE_DEEP_SLEEP 1

#endif // CONFIG_H

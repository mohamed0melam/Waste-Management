# ESP32 A0221AU PlatformIO patched project

This project contains a patched version of your firmware with the following changes:
- Device instance created inside setup() to avoid early boot watchdog resets on ESP32-C3.
- WiFi loops call yield() to avoid watchdog during blocking waits.
- MQTT publish uses explicit payload length and increased MQTT_MAX_PACKET_SIZE = 1024.
- TimeManager.waitForTime() commented out for early debugging.
- SENSOR_RX_PIN default set to GPIO9 (change to your actual pin).

Build with PlatformIO (ensure env matches your board):
```
pio run -e esp32-c3-devkitm-1 -t upload
pio device monitor -e esp32-c3-devkitm-1
```

Requirements: ArduinoJson, PubSubClient libraries (lib_deps set in platformio.ini).

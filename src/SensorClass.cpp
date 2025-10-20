#include "SensorClass.h"
#include <HardwareSerial.h>

static HardwareSerial &SensorSerial = Serial1;

SensorClass::SensorClass(int rxPin) : _rxPin(rxPin) {}

void SensorClass::begin() {
  if (_rxPin < 0) return;
  SensorSerial.begin(9600, SERIAL_8N1, _rxPin, -1);
  delay(50);
  Serial.printf("📡 Sensor initialized on RX pin %d\n", _rxPin);
}

float SensorClass::readDistance() {
  if (_rxPin < 0) return -1.0f;
  if (SensorSerial.available() < 4) { delay(50); return -1.0f; }
  int header = SensorSerial.read() & 0xFF;
  if (header != 0xFF) return -1.0f;
  int high = SensorSerial.read() & 0xFF;
  int low  = SensorSerial.read() & 0xFF;
  int sum  = SensorSerial.read() & 0xFF;
  int calc = (0xFF + high + low) & 0xFF;
  if (calc != sum) return -1.0f;
  int dist_mm = (high << 8) | low;
  return dist_mm / 10.0f; // cm
}

float SensorClass::readValidatedMedian() {
  const int N = 10;
  float arr[N];
  int valid = 0;
  for (int i = 0; i < N; ++i) {
    float d = readDistance();
    if (d > 0) arr[valid++] = d;
    delay(100);
  }
  Serial.printf("Validation: %d/%d valid readings\n", valid, N);
  if (valid < 5) return -1.0f;
  for (int i = 0; i < valid-1; ++i)
    for (int j = i+1; j < valid; ++j)
      if (arr[j] < arr[i]) { float t = arr[i]; arr[i] = arr[j]; arr[j] = t; }
  return arr[valid/2];
}

#ifndef SENSOR_CLASS_H
#define SENSOR_CLASS_H

#include <Arduino.h>

class SensorClass {
public:
  explicit SensorClass(int rxPin);
  void begin();
  float readDistance();
  float readValidatedMedian();
private:
  int _rxPin;
};

#endif // SENSOR_CLASS_H

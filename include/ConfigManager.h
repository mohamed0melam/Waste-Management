#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include <Arduino.h>

class ConfigManager {
public:
  ConfigManager();
  void begin();                     // load persisted config if any
  // applyFromJson: returns true if JSON parsed successfully (regardless of whether it changed)
  bool applyFromJson(const String& json);

  // after applyFromJson you can call this to know if a new config was persisted
  bool wasLastApplyChanged() const { return _lastApplyChanged; }

  uint32_t getPollingMs() const;
  float    getBinHeightCm() const;
  float    getBinWidthCm() const;
  float    getBinLengthCm() const;

  static void    setPollingRateSec(uint32_t s); // sets and persists
  static uint32_t getPollingRateSec();

private:
  uint32_t _pollingMs;
  float    _binHeightCm;
  float    _binWidthCm;
  float    _binLengthCm;

  // runtime polling value; 0 = not set (use default)
  static uint32_t s_pollingRateSec;

  // fallback default if nothing is present
  static const uint32_t s_defaultPollingRateSec;

  // result flag set for last applyFromJson()
  bool _lastApplyChanged;

  // tolerance for comparing floating values (cm)
  static constexpr float FLOAT_EPS = 0.01f;
};

#endif // CONFIG_MANAGER_H
